#include "Flakes.cpp"

int numBoxAddr = 0, modeAddr = numBoxAddr + INT_OFF, pAddr = modeAddr + INT_OFF,
    sAddr = pAddr + STR_OFF, briAddr = sAddr + STR_OFF,
    ipAddr = briAddr + BYTE_OFF;
int addr = 0;
int modeSelected = -1;
bool prev_input = false;
String ip_str;
const String deviceId = "54545"; // automate

ESP8266WebServer server(80);
Flakes *panel = new Flakes();

// wifimanager configModeCallback

void configModeCallback(WiFiManager *wm) {

  //EEPROM.wipe();
  // Serial.println("Entered Config Mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(wm->getConfigPortalSSID());
  // TODO: blink panels for indication
}

void handleRoot() {
  //  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from FLAKE: " + deviceId);
  //  digitalWrite(led, 0);
}

void sendResponse(String response) {
  DynamicJsonDocument doc(2048);
  doc["response"] = response;
  String json;
  serializeJson(doc, json);
  server.send(200, "text/plain", json);
}

void handleNotFound() {
  //  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {

  pinMode(PIN, OUTPUT);
  //EEPROM.begin(sizeof(Flakes));
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setAPCallback(configModeCallback);
  panel->leds = new CRGB[panel->total_led];
  bool res = wm.autoConnect();
  // if (EEPROM.percentUsed() > 0) {
  //   IPAddress local_ip(active.ip[0], active.ip[1], active.ip[2],
  //   active.ip[3]); IPAddress gateway(192, 168, 1, 1); IPAddress subnet(255,
  //   255, 0, 0);
  // }
  if (!res) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("Connected to: ");
  }
  Serial.print(" ");
  Serial.println(WiFi.SSID());
  Serial.print(" IP address: ");
  Serial.println(WiFi.localIP());

  FastLED.addLeds<LED_TYPE, PIN, COLOR_ORDER>(panel->leds, panel->total_led);
  Serial.println("Setting blue");
  for(int i=0; i<panel->total_led; i++){
      panel->leds[i] = CRGB(0,255,0);
    }
    FastLED.show();
  server.on("/", handleRoot);

  // Request for verificatin of the device
  server.on("/verification", []() {
    if (server.hasArg("id")) {
      if (server.arg("id") == deviceId) {
        // Serial.println("ID MATCH!");
        panel->num_of_boxes = server.arg("numBox").toInt();
       // EEPROM.put(numBoxAddr, panel->num_of_boxes);
        //EEPROM.put(ipAddr, WiFi.localIP()); // store ip in eeprom
        bool res =1;// EEPROM.commit();
        if (res)
          sendResponse(deviceId);
        else {
          sendResponse("ok");
        }
      }
    }
    server.onNotFound(handleNotFound);
  });

  // Request for static mode
  server.on("/sm", []() {
    if (server.hasArg("id") && server.hasArg("Scolor") &&
        server.hasArg("Sbrightness")) {
      if (server.arg("id") == deviceId) {
        // use Scolor and Sbrigthness
        // optimise ? make generic color parsing functions
        modeSelected = 0;
        panel->setMode(
          modeAddr,
          modeSelected); // sets panel->mode_active and stores in eeprom
        panel->rgbValueUpdate(server.arg("Scolor"), 0);
        panel->brightness = panel->parseBrightness(server.arg("Sbrightness"));
//        EEPROM.put(briAddr, panel->brightness);
//        EEPROM.commit();
        sendResponse("ok");
      }
    }
    server.onNotFound(handleNotFound);
  });

  // Request for Twinkle Mode
  server.on("/tm", []() {
    if (server.hasArg("id") && server.hasArg("Pcolor") &&
        server.hasArg("Scolor") && server.hasArg("Tbrightness")) {
      if (server.arg("id") == deviceId) {
        // use primary and secondary color and their respective brightness
        modeSelected = 1;
        panel->setMode(modeAddr, modeSelected);
        panel->rgbValueUpdate(server.arg("Pcolor"), 0);
        panel->rgbValueUpdate(server.arg("Scolor"), 1);
        panel->brightness = panel->parseBrightness(server.arg("Tbrightness"));
//        EEPROM.put(briAddr, panel->brightness);
//        EEPROM.commit();
        sendResponse("ok");
      }
    }
    server.onNotFound(handleNotFound);
  });

  // Request for Gradient Mode
  server.on("/gm", []() {
    if (server.hasArg("id") && server.hasArg("Pcolor") &&
        server.hasArg("Scolor") && server.hasArg("Gbrightness")) {
      if (server.arg("id") == deviceId) {
        modeSelected = 3;
        panel->setMode(modeAddr, modeSelected);
        panel->rgbValueUpdate(server.arg("Pcolor"), 0);
        panel->rgbValueUpdate(server.arg("Scolor"), 1);
        panel->brightness = panel->parseBrightness(server.arg("Gbrightness"));
//        EEPROM.put(briAddr, panel->brightness);
//        EEPROM.commit();
        sendResponse("ok");
      }
    }
    server.onNotFound(handleNotFound);
  });

  // Request for Party Mode
  server.on("/pm", []() {
    if (server.hasArg("id")) {
      if (server.arg("id") == deviceId) {
        modeSelected = 5;
        panel->setMode(modeAddr, modeSelected);
//        EEPROM.put(briAddr, panel->brightness);
//        EEPROM.commit();
        sendResponse("ok");
      }
    }
    server.onNotFound(handleNotFound);
  });

  // request for the night mode
  server.on("/nm", []() {
    if (server.hasArg("id") && server.hasArg("Scolor") &&
        server.hasArg("Sbrightness")) {
      if (server.arg("id") == deviceId) {
        modeSelected = 4;
        panel->setMode(modeAddr, modeSelected);
        panel->rgbValueUpdate(server.arg("Scolor"), 0);
        panel->brightness = panel->parseBrightness(server.arg("Sbrightness"));
//        EEPROM.put(briAddr, panel->brightness);
//        EEPROM.commit();
          sendResponse("ok");
      }
    }
  });

  // request for  the diverse mode

  server.on("/dm", []() {
    if (server.hasArg("id") && server.hasArg("Colors") &&
        server.hasArg("Dbrightness")) {
      if (server.arg("id") == deviceId) {
        modeSelected = 2;
        panel->setMode(modeAddr, modeSelected);
        panel->diverse_color = server.arg("Colors");
        panel->brightness = panel->parseBrightness(server.arg("Dbrightness"));
//        EEPROM.put(briAddr, panel->brightness);
//        EEPROM.commit();
        sendResponse("ok");
      }
    }
  });
  server.on("/brightness", []() {
    if (server.hasArg("id") && server.hasArg("brightness")) {
      if (server.arg("id") == deviceId) {
        Serial.println(server.arg("brightness"));
        panel->brightness = panel->parseBrightness(server.arg("brightness"));
        FastLED.setBrightness(panel->brightness);
        FastLED.show();

        sendResponse("ok");
      }
    }
    sendResponse("invalid");
  });

  // Unhandled
  server.onNotFound(handleNotFound);

//  bool eeprom_c = EEPROM.commit();
//  Serial.println((eeprom_c) ? "EEPROM write successful!"
//                 : "EEPROM write failed!");

  server.begin();
}

void loop(void) {
  delay(10);
  server.handleClient();
  if (modeSelected == 0) { //static
    panel->staticAndNightMode(panel->leds);
  }
  else if (modeSelected == 1) { //twinkle
    panel->twinkleMode(panel->leds);
  }
  else if (modeSelected == 2) {
    panel->diverseMode(panel->leds);
  }
  else if (modeSelected == 3) {
    panel->gradientMode(panel->leds);
  }
  else if (modeSelected == 4) {
    panel->staticAndNightMode(panel->leds);
  }
  else if (modeSelected == 5) {
    panel->partyMode(panel->leds);
  }
}
