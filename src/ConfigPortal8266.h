/*
 *  ConfigPortal library to extend and implement the WiFi connected IOT device
 *
 *  Yoonseok Hur
 *
  *  Usage Scenario:
 *  0. copy the example template in the README.md
 *  1. Modify the ssid_pfix to help distinquish your Captive Portal SSID
 *          char   ssid_pfix[];
 *  2. Modify user_config_html to guide and get the user config data through the Captive Portal
 *          String user_config_html;
 *  2. declare the user config variable before setup
 *  3. In the setup(), read the cfg["meta"]["your field"] and assign to your config variable
 *
 */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#define             JSON_BUFFER_LENGTH 1024
StaticJsonDocument<JSON_BUFFER_LENGTH> cfg;

ESP8266WebServer    webServer(80);
const int           RESET_PIN = 0;

char                cfgFile[] = "/config.json";

extern              String user_config_html;
extern char         *ssid_pfix;

String html_begin = ""
    "<html><head><title>IOT Device Setup</title></head>"
    "<body><center><h1>Device Setup Page</h1>"
        "<style>"
            "input {font-size:3em; width:90%; text-align:center;}"
            "button { border:0;border-radius:0.3rem;background-color:#1fa3ec;"
            "color:#fff; line-height:2em;font-size:3em;width:90%;}"
        "</style>"
        "<form action='/save'>"
            "<p><input type='text' name='ssid' placeholder='SSID'>"
            "<p><input type='text' name='w_pw'placeholder='Password'>";

String html_end = ""
            "<p><button type='submit'>Save</button>"
        "</form>"
    "</center></body></html>";

String postSave_html_default = ""
    "<html><head><title>Reboot Device</title></head>"
    "<body><center><h5>Device Configuration Finished</h5><h5>Click the Reboot Button</h5>"
        "<p><button type='button' onclick=\"location.href='/reboot'\">Reboot</button>"
    "</center></body></html>";

String redirect_html = ""
    "<html><head><meta http-equiv='refresh' content='0; URL=http:/pre_boot' /></head>"
    "<body><p>Redirecting</body></html>";

String postSave_html;

void (*userConfigLoop)() = NULL;

void byte2buff(char* msg, byte* payload, unsigned int len) {
    unsigned int i, j;
    for (i=j=0; i < len ;) {
        msg[j++] = payload[i++];
    }
    msg[j] = '\0';
}

void getULongValue(JsonObject &o, char* n, unsigned long* v) {
    if (o.containsKey(n)) {
        *v = 0;
        JsonVariant m = o.getMember(n);
        if(m.is<String>()) {
            String s = m.as<String>();
            if (s.length() > 0) {
                *v = m.as<String>().toInt();
            }
        } else if(m.is<int>()) { 
            *v = m.as<int>();
        }
    }
}

void getIntValue(JsonObject &o, char* n, int* v) {
    if (o.containsKey(n)) {
        *v = 0;
        JsonVariant m = o.getMember(n);
        if(m.is<String>()) {
            String s = m.as<String>();
            if (s.length() > 0) {
                *v = m.as<String>().toInt();
            }
        } else if(m.is<int>()) { 
            *v = m.as<int>();
        }
    }
}

void getFloatValue(JsonObject &o, char* n, float* v) {
    *v = 0;
    if (o.containsKey(n)) {
        JsonVariant m = o.getMember(n);
        if(m.is<String>()) {
            String s = m.as<String>();
            if (s.length() > 0) {
                *v = m.as<String>().toFloat();
            }
        } else if(m.is<float>()) { 
            *v = m.as<float>();
        }
    }
}

void save_config_json(){
    char cfgBuffer[JSON_BUFFER_LENGTH];
    serializeJson(cfg, cfgBuffer);
    File f = LittleFS.open(cfgFile, "w");
    f.print(cfgBuffer);
    f.close();
}

void reset_config() {
	deserializeJson(cfg, "{meta:{}}");
    save_config_json();
}

void maskConfig(char* buff) {
    DynamicJsonDocument temp_cfg = cfg;
    if(cfg.containsKey("w_pw")) temp_cfg["w_pw"] = "********";
    if(cfg.containsKey("token")) temp_cfg["token"] = "********";
    serializeJson(temp_cfg, buff, JSON_BUFFER_LENGTH);
}

IRAM_ATTR void reboot() {
    WiFi.disconnect();
    ESP.restart();
}

void loadConfig() {
    // check Factory Reset Request and reset if requested or load the config
    if(!LittleFS.begin()) { LittleFS.format(); }    // before the reset_config and reading

    pinMode(RESET_PIN, INPUT_PULLUP);
    if( digitalRead(RESET_PIN) == 0 ) {
        unsigned long t1 = millis();
        while(digitalRead(RESET_PIN) == 0) {
            delay(500);
            Serial.print(".");
        }
        if (millis() - t1 > 5000) {
            reset_config();             // Factory Reset
        }
    }
    attachInterrupt(RESET_PIN, reboot, FALLING);

    if (LittleFS.exists(cfgFile)) {
        File f = LittleFS.open(cfgFile, "r");
        char buff[512];
        int i = 0;
        while(f.available()) {
            buff[i++] = f.read();
        }
        f.close();

        DeserializationError error = deserializeJson(cfg, String(buff));
	    if (error) {
	        deserializeJson(cfg, "{meta:{}}");
	    } else {
	        Serial.println("CONFIG JSON Successfully loaded");
	        char maskBuffer[JSON_BUFFER_LENGTH];
	        maskConfig(maskBuffer);
	        Serial.println(String(maskBuffer));
	    }
    } else {
	    deserializeJson(cfg, "{meta:{}}");
    }
}

void saveEnv() {
    int args = webServer.args();
    for (int i = 0; i < args ; i++){
        if (webServer.argName(i).indexOf(String("meta.")) == 0 ) {
            String temp = webServer.arg(i);
            temp.trim();
            cfg["meta"][webServer.argName(i).substring(5)] = temp;
        } else {
            String temp = webServer.arg(i);
            temp.trim();
            cfg[webServer.argName(i)] = temp;
        }
    }
    cfg["config"] = "done";
    save_config_json();
    // redirect uri augmentation here
    //
    webServer.send(200, "text/html", redirect_html);
}

void pre_reboot() {
    int args = webServer.args();
    for (int i = 0; i < args ; i++){
        Serial.printf("%s -> %s\n", webServer.argName(i).c_str(), webServer.arg(i).c_str());
    }
    webServer.send(200, "text/html", postSave_html);
}

bool getHTML(String* html, char* fname) {
    if (LittleFS.exists(fname)) {
        File f = LittleFS.open(fname, "r");
        char buff[20480];
        int i = 0;
        while(f.available()) {
            buff[i++] = f.read();
        }
        buff[i] = '\0';
        f.close();
        *html = String(buff);
        return true;
    } else {
        return false;
    }
}

void configDevice() {
    DNSServer   dnsServer;
    const byte  DNS_PORT = 53;
    IPAddress   apIP(192, 168, 1, 1);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    char ap_name[100];
    sprintf(ap_name, "%s_%08X", ssid_pfix, ESP.getChipId());
    WiFi.softAP(ap_name);
    dnsServer.start(DNS_PORT, "*", apIP);

    if (getHTML(&postSave_html, (char*)"/postSave.html")) {
        // argument redirection 
    } else {
        postSave_html = postSave_html_default;
    }

    webServer.on("/save", saveEnv);
    webServer.on("/reboot", reboot);
    webServer.on("/pre_boot", pre_reboot);

    webServer.onNotFound([]() {
        webServer.send(200, "text/html", html_begin + user_config_html + html_end);
    });
    webServer.begin();
    Serial.println("starting the config");
    while(1) {
        yield();
        dnsServer.processNextRequest();
        webServer.handleClient();
        if(userConfigLoop != NULL) {
            (*userConfigLoop)();
        }
    }
}