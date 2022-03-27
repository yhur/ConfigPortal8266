# Config Portal

With this library, the developer can create a WiFi ESP8266 device which 
1. on power on, it provides with the Captive Portal if not configured, where the user can enter the configuration information such as SSID/password, and save to the ESP8266 flash filesystem.
2. or boots with the stored SSID/password and other programmed information if configured already.
3. connects to the WiFi and run the programmed setup function
4. it will serve as written in the loop function
5. (factory reset) if needed, you can connect GPIO0 to ground for more than 5 seconds, then the configuration will be deleted and the device can be reconfigured for the WiFi.

This is the screenshot of a sample captive portal.

<img width="381" alt="Captive Portal" src="https://user-images.githubusercontent.com/13171662/150663373-ffb1d4e9-e128-41a6-9231-3cb9f6f495f7.png">

# How to use the ConfigPortal8266
You can create a PlatformIO project with the example directory and modify the src/main.cpp for your purpose and build it.

## src/main.cpp 
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

## customization
If additional configuration parameter is needed, you can modify the `user_config_html` for your variable, and handle the variable as shown below. The custom varialbe here is yourVar in the `user_html` and handling in setup code as below

In the global section.
```c
String              user_config_html = ""
    "<p><input type='text' name='yourVar' placeholder='Your Variable'>";
```

In the functions.
```c
    Serial.println( (const char*)cfg["yourVar"]);
```

