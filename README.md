# Config Portal

With this library, the developer can create a WiFi ESP8266 device which provides with the Captive Portal if not configured, where the user can enter the configuration information and save to the ESP8266 flash filesystem.

On next and following boot, the stored information will used to set up the WiFi connection and other settings for the device.

The following code is the example to use the library. 
```c
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ConfigPortal8266.h>

char*               ssid_pfix = (char*)"CaptivePortal";
String              user_config_html = "";      

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
    // main setup
    Serial.printf("\nIP address : "); Serial.println(WiFi.localIP());

    if (MDNS.begin("miniwifi")) {
        Serial.println("MDNS responder started");
    }    
}

void loop() {
    MDNS.update();
}
```

If additional configuration parameter is needed, you can modify the `user_html` as above, and handle the variable as above example. The custom varialbe here is yourVar in the `user_html` and handling in setup code as below

```c
    Serial.println( (const char*)cfg["yourVar"]);
```

