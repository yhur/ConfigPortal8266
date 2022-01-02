Config Portal
This library provides with the Captive Portal where the user can enter the configuration information, and it saves to the ESP8266 flash filesystem.
On next and following boot, the stored information will used to set up the devic

```c
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ConfigPortal.h>

String          user_html = ""
                "<p><input type='text' name='yourVar' placeholder='Your Variable'>";
char*           ssid_pfix = (char*)"CaptivePortal";

void setup() {
    Serial.begin(115200);
    loadConfig();
    // *** If no "config" is found or "config" is not "done", run configDevice ***
    if(!cfg.containsKey("config") || strcmp((const char*)cfg["config"], "done")) {
        configDevice();
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nIP address : "); Serial.println(WiFi.localIP());
    char cfgBuffer[512];
    maskConfig(cfgBuffer);
    Serial.println(cfgBuffer);
    Serial.println( (const char*)cfg["yourVar"]);
    // main setup
}

void loop() {
}
```
