#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#ifndef STASSID
#define STASSID "ARCHE_INDUSTRIES"
#define STAPSK  "123456789"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
const char* deviceId = "54545";


ESP8266WebServer server(80);


#define PIN 4
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_OF_LEDS 30
#define NUM_OF_BOX 6

CRGB leds[NUM_OF_BOX*NUM_OF_LEDS];

int r=0,g=0,b=0;

int getValue(char a)
{
    if(a=='a'){return 10;}
    else if(a=='b'){return 11;}
    else if(a=='c'){return 12;}
    else if(a=='d'){return 13;}
    else if(a=='e'){return 14;}
    else if(a=='f'){return 15;}
    else if(a=='1'){return 1;}
    else if(a=='0'){return 0;}
    else if(a=='2'){return 2;}
    else if(a=='3'){return 3;}
    else if(a=='4'){return 4;}
    else if(a=='5'){return 5;}
    else if(a=='6'){return 6;}
    else if(a=='7'){return 7;}
    else if(a=='8'){return 8;}
    else if(a=='9'){return 9;}
}



int value(char a, char b) 
{
  return (16*(getValue(a)) + 1*(getValue(b)));
}





void handleRoot() {
//  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
//  digitalWrite(led, 0);
}


void sendResponse(String response){
      DynamicJsonDocument doc(2048);
      doc["response"] = response;
      String json;
      serializeJson(doc,json);
      server.send(200,"text/plain",json);
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
//  digitalWrite(led, 0);
}


void setup() {
  
  
  pinMode(PIN, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  FastLED.addLeds<LED_TYPE, PIN, COLOR_ORDER>(leds, NUM_OF_LEDS*NUM_OF_BOX);
 

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
  server.on("/verification",[]()
  {
    if(server.hasArg("id"))
    {
      if(server.arg("id")==deviceId)
      { Serial.println("got the signal");
         sendResponse(deviceId);
      } 
    }
       server.onNotFound(handleNotFound);
  });


  
// Request for static mode
  server.on("/sm",[]()
  {
    if(server.hasArg("id")&& server.hasArg("Scolor")&& server.hasArg("Sbrightness"))
    {
      if(server.arg("id")==deviceId)
      {
        //use Scolor and Sbrigthness
        String Scolor = server.arg("Scolor");
        int len = Scolor.length();
        char data[len];
        Scolor.toCharArray(data,len);
         r = value(data[0],data[1]);
         g = value(data[2],data[3]);
         b = value(data[4],data[5]);
       
        
        sendResponse("ok");  
      }
    }
    server.onNotFound(handleNotFound);
  });

  
// Request for Twinkle Mode
  server.on("/tm",[]()
  {
    if(server.hasArg("id")&&server.hasArg("Pcolor")&&server.hasArg("Scolor")&&server.hasArg("Tbrightness"))
    {
      if(server.arg("id")==deviceId)
      {
      //use primary and secondary color and their respective brightness
        sendResponse("ok");
      }
    }
    server.onNotFound(handleNotFound);
  });

  
// Request for Gradient Mode
  server.on("/gm",[]()
  {
    if(server.hasArg("id")&&server.hasArg("Pcolor")&&server.hasArg("Scolor")&&server.hasArg("Gbrightness"))
    {
      if(server.arg("id")==deviceId)
      {
      //use primary and secondary color and their respective brightness
        sendResponse("ok");
      }
    }
    server.onNotFound(handleNotFound);
  });


// Request for Party Mode
  server.on("/pm",[]()
  {
    if(server.hasArg("id"))
    {
      if(server.arg("id")==deviceId)
      {
        //initiate party mode
        sendResponse("ok");
      }
    }
    server.onNotFound(handleNotFound);
  });


  // Unhandled
  server.onNotFound(handleNotFound);


  server.begin();
//  Serial.println("HTTP server started");
}


void loop(void) {
  server.handleClient();
   for(int i=0;i<30;i++)
        {
          leds[i] = CRGB(r,g,b);
        }
        FastLED.show();
        delay(80);
}
