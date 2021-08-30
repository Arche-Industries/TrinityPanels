//add webserver
//mode implementation

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include<EEPROM.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include<WiFiManager.h>


#define PIN 4
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_OF_LEDS 30
#define INT_OFF 4
#define STR_OFF 8 
#define BYTE_OFF 4
//preset struct
struct typedef{
  int num_of_boxes;
  String pColor;
  String sColor;
  int modeS;
  int brightness;
  byte ip[4];
}preset;

preset active;

int numBoxAddr = 0, modeAddr=numBoxAddr+INT_OFF, pAddr = modeAddr+INT_OFF, sAddr = pAddr + STR_OFF, briAddr = sAddr+STR_OFF, ipAddr = briAddr + BYTE_OFF; 
int addr = 0;
int modeSelected = -1;
String ip_str;
const char* ssid = STASSID;
const char* password = STAPSK;
const char* deviceId = "54545";


ESP8266WebServer server(80);



CRGB leds[NUM_OF_BOX * NUM_OF_LEDS];

int r = 0, g = 0, b = 0, bri = 0;

void parseIP(){
  String ip = WiFi.localIP();
  int len = ip.length();
  char ip_arr[len];
  ip.toCharArray(ip_arr,len);
  int num = 0;
  int counter = 3;
  int mul = 1;
  char tmp = ip_arr[0];
  for(int i = len-1 ; i > -1; i--){
      if(tmp[i]!='.'){
        num += mul * getValue(tmp[i]);
        mul = mul*10;
      }
      else{
        active.ip[counter] = num;
        mul = 1;
        num = 0;
        counter--;
      }
    }
    EEPROM.put(ipAddr,active.ip);
  }
//store mode in eeprom and in preset instance
void setMode(int addr){
     active.modeS = modeSelected;
     EEPROM.put(addr,active.modeS);
  }
  
//party mode
void partyMode() {
  //delay(50);
  Serial.println("generating colors");
  r = random(30, 225);
  g = random(30, 225);
  b = random(30, 225);
  for(int i=0;i<NUM_OF_LEDS*NUM_OF_BOX;i++)
  {
    leds[i] = CRGB(r,g,b);
    
  }
  
  bri = random(200,700);
  int tmp = 50;
  while(tmp<=bri){
      FastLED.setBrightness(tmp);
      FastLED.show();
      tmp+=40;
      delay(100);
    }
    //delay(500);
  r = random(30, 225);
  g = random(30, 225);
  b = random(30, 225);
  for(int i=0;i<NUM_OF_LEDS*NUM_OF_BOX;i++)
  {
    leds[i] = CRGB(r,g,b);
    
  }
  while(bri>=50){
  
      FastLED.setBrightness(bri);
      Serial.println(bri);
      FastLED.show();
      bri-=40;
      delay(100);
    }
  
  
  
}

//optimise
int getValue(char a)
{
  if (a == 'a') {
    return 10;
  }
  else if (a == 'b') {
    return 11;
  }
  else if (a == 'c') {
    return 12;
  }
  else if (a == 'd') {
    return 13;
  }
  else if (a == 'e') {
    return 14;
  }
  else if (a == 'f') {
    return 15;
  }
  else if (a == '1') {
    return 1;
  }
  else if (a == '0') {
    return 0;
  }
  else if (a == '2') {
    return 2;
  }
  else if (a == '3') {
    return 3;
  }
  else if (a == '4') {
    return 4;
  }
  else if (a == '5') {
    return 5;
  }
  else if (a == '6') {
    return 6;
  }
  else if (a == '7') {
    return 7;
  }
  else if (a == '8') {
    return 8;
  }
  else if (a == '9') {
    return 9;
  }
}



int value(char a, char b)
{
  return (16 * (getValue(a)) + 1 * (getValue(b)));
}



/*
  Callbacks
*/
//wifimanager configModeCallback

void configModeCallback(WiFiManager* wm){

    EEPROM.wipe();
    Serial.println("Entered Config Mode");
    Serial.println(WiFi.softAPIP());
    Serial.println(wm->getConfigPortalSSID());
    //TODO: blink panels for indication
  }

void handleRoot() {
  //  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
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
  EEPROM.begin(sizeof(preset));
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setAPCallback(configModeCallback);
  
  bool res = wm.autoConnect();
  if(EEPROM.percentageUsed()>0){
    IPAddress local_ip(active.ip[0],active.ip[1],active.ip[2],active.ip[3]);
    IPAddress gateway(192,168,1,1);
    IPAddress subnet(255,255,0,0);
  }
  if(!res){
    
    Serial.println("Failed to connect");
  }
  else{
    Serial.println("Connected to: ");
  }
  Serial.print(" ");
  Serial.println(WiFi.SSID());
  Serial.print(" IP address: ");
  Serial.println(WiFi.localIP());
  
  FastLED.addLeds<LED_TYPE, PIN, COLOR_ORDER>(leds, NUM_OF_LEDS * NUM_OF_BOX);


  server.on("/", handleRoot);

  // for get requests with data onto the server
  //  server.on("/test",[](){
  //    int argument_len = server.args();
  //    Serial.println(argument_len);
  //    for(uint8_t i=0;i<argument_len;i++)
  //    {
  //      Serial.print("Argument name : ");
  //      Serial.println(server.argName(i));
  //      Serial.print("Arguments :");
  //      Serial.println(server.arg(i));
  //      if(server.argName(i)=="param1")
  //      {
  //        Serial.println("its param1");
  //      }
  //    }






  // Request for verificatin of the device
  server.on("/verification", []()
  {
    if (server.hasArg("id"))
    {
      if (server.arg("id") == deviceId)
      { Serial.println("got the signal");
        active.num_of_boxes = server.arg("numBox");
        EEPROM.put(numBoxAddr,active.num_of_boxes);
        parseIP();                //store ip in eeprom
        sendResponse(deviceId);
      }
    }
    server.onNotFound(handleNotFound);
  });



  // Request for static mode
  server.on("/sm", []()
  {
    if (server.hasArg("id") && server.hasArg("Scolor") && server.hasArg("Sbrightness"))
    {
      if (server.arg("id") == deviceId)
      {
        //use Scolor and Sbrigthness
        //optimise ? make generic color parsing functions
        String Scolor = server.arg("Scolor");
        int len = Scolor.length();
        char data[len];
        Scolor.toCharArray(data, len);
        r = value(data[0], data[1]);
        g = value(data[2], data[3]);
        b = value(data[4], data[5]);
        modeSelected = 0;
        setMode(modeAddr);
        sendResponse("ok");
      }
    }
    server.onNotFound(handleNotFound);
  });


  // Request for Twinkle Mode
  server.on("/tm", []()
  {
    if (server.hasArg("id") && server.hasArg("Pcolor") && server.hasArg("Scolor") && server.hasArg("Tbrightness"))
    {
      if (server.arg("id") == deviceId)
      {
        //use primary and secondary color and their respective brightness
        modeSelected = 1;
        setMode(modeAddr);
        sendResponse("ok");
      }
    }
    server.onNotFound(handleNotFound);
  });


  // Request for Gradient Mode
  server.on("/gm", []()
  {
    if (server.hasArg("id") && server.hasArg("Pcolor") && server.hasArg("Scolor") && server.hasArg("Gbrightness"))
    {
      if (server.arg("id") == deviceId)
      { 
        modeSelected = 3;
        setMode(modeAddr);
        //use primary and secondary color and their respective brightness
        sendResponse("ok");
      }
    }
    server.onNotFound(handleNotFound);
  });


  // Request for Party Mode
  server.on("/pm", []()
  {
    if (server.hasArg("id"))
    {
      if (server.arg("id") == deviceId)
      {
        //initiate party mode
        modeSelected = 5;
        setMode(modeAddr);
        Serial.println("send ok for pm");
        sendResponse("ok");
        
      }
    }
    server.onNotFound(handleNotFound);
  });


  // request for the night mode 
  server.on("/nm",[](){
    
    if(server.hasArg("id")&&server.hasArg("Scolor")&&server.hasArg("Sbrightness")){
      if(server.arg("id")==deviceId){
        modeSelected = 4;        // main code here
        setMode(modeAddr);
        //sendResponse("ok"); //send response or not?
        }
      }
    
    });

  // request for  the diverse mode

  server.on("/dm", []() {
    if (server.hasArg("id") && server.hasArg("Colors") && server.hasArg("Dbrightness"))
    {
      if (server.arg("id") == deviceId) {
        // put a parser here
        modeSelected = 2;
        setMode(modeAddr);
        sendResponse("ok");
      }
  
    }
  });


  // Unhandled
  server.onNotFound(handleNotFound);

  bool eeprom_c = EEPROM.commit();
  Serial.println((eeprom_c) ? "EEPROM write successful!" : "EEPROM write failed!");

  server.begin();
}


void loop(void) {
  server.handleClient();


  // idle mode
  if (modeSelected == -1) {
      //if mode==-1   i.e. app didnt connect or no mode was selected and if there is any previously stored preset.
      if(EEPROM.percentUsed()>=0){
          //set mode to what is stored in eeprom.
          EEPROM.get(modeAddr,modeSelected);
        }
      else{
          Serial.println("no mode selected or saved");
        }    
    }
  // static mode
  else if (modeSelected == 0) {
    for (int i = 0; i < 30; i++)
    {
      leds[i] = CRGB(r, g, b);
    }
    FastLED.show();
    delay(80);
  }
  // twinkle mode
  else if (modeSelected == 1) {
    }
  // diverse mode
  else if (modeSelected == 2) {
    }
  // Gradient Mode
  else if (modeSelected == 3) {}
  // Night Mode
  else if (modeSelected == 4) {}
  // Party Mode
  else if (modeSelected == 5) {
    Serial.println("pm ");
    partyMode();
  }
  // Music Mode
  else if (modeSelected == 6) {}
}
