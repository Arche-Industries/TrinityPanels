/*--------------------INCLUDES--------------*/
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
//#include <ESP_EEPROM.h>
#include <FastLED.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
/*------------------------------------------*/

/*---------------------DEFINES--------------*/

#define PIN 4
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_OF_LEDS 30
#define INT_OFF 4
#define STR_OFF 8
#define BYTE_OFF 4
#define NUM_OF_BOX 6 // get this from server
#define R 0
#define G 1
#define B 2
/*-----------------------------------------*/

class Flakes {
public:
  Flakes() {
    // do nothing
  }
  int num_of_boxes = 6; // default:6
  int mode_active;
  int brightness = 105;
  int total_led = NUM_OF_LEDS * num_of_boxes;
  uint8_t primary[3];
  uint8_t secondary[3];
  String diverse_color = "";
  IPAddress ip;
  CRGB *leds;
  int getValue(char a) {
    if (a == 'a') {
      return 10;
    } else if (a == 'b') {
      return 11;
    } else if (a == 'c') {
      return 12;
    } else if (a == 'd') {
      return 13;
    } else if (a == 'e') {
      return 14;
    } else if (a == 'f') {
      return 15;
    } else if (a == '1') {
      return 1;
    } else if (a == '0') {
      return 0;
    } else if (a == '2') {
      return 2;
    } else if (a == '3') {
      return 3;
    } else if (a == '4') {
      return 4;
    } else if (a == '5') {
      return 5;
    } else if (a == '6') {
      return 6;
    } else if (a == '7') {
      return 7;
    } else if (a == '8') {
      return 8;
    } else if (a == '9') {
      return 9;
    }
  }
  /*
  PARAM:
    r,g,b colors and array to store converted array
    RETURN:
      void
     converts rgb to hsv stores them in buff provided.
  */
  void rgb2hsv(uint8_t r,uint8_t g, uint8_t b, uint16_t hsv_arr){
    float h,s,v;
    float cmin,cmax,delta;
    float rp,gp,bp;
    float maxVal = 255.0;

    rp = float(r/maxVal);
    gp = float(g/maxVal);
    bp = float(b/maxVal);
    std::cout<<"\n";
    cmax = rp>gp?(rp>bp?rp:bp):(gp>bp?gp:bp);

    cmin = rp<gp?(rp<bp?rp:bp):(gp<bp?gp:bp);

    delta = cmax - cmin;


    s = cmax==0?0:(delta/cmax)*100;

    v = cmax*100;

    h = delta==0?0:cmax==rp?(int((gp-bp)/delta)%6):cmax==gp?(((bp-rp)/delta)+2):(((rp-gp)/delta)+4);
    h = h*60;
    if(h<0){
        h = 360+h;
    }
    hsv_arr[0] = (int)(h);
    hsv_arr[1] = (int)(s);
    hsv_arr[2] = (int)(v);
   }
  int value(char a, char b) { return (16 * (getValue(a)) + 1 * (getValue(b))); }
  /*
  PARAM:
      addr: offset to store mode after
      mode: active mode
  RETURN: void
  sets the active mode in panel instance and in eeprom
  */
  void setMode(int addr, int mode) {
    mode_active = mode;
  //  EEPROM.put(addr, mode);
  }

  /*
  PARAM:
      color: the color to parse
      toggle: color is to be parsed into primary or secondary array
  RETURN:
      void
  Parses the provided color to appropriate color array
  */
  void rgbValueUpdate(String color, bool toggle) {
    String color_s = color;
    int len = color_s.length();
    char data[len];
    color_s.toCharArray(data, len);
    if (!toggle) {
      primary[R] = value(data[0], data[1]);
      primary[G] = value(data[2], data[3]);
      primary[B] = value(data[4], data[5]);
    } else {
      secondary[R] = value(data[0], data[1]);
      secondary[G] = value(data[2], data[3]);
      secondary[B] = value(data[4], data[5]);
    }
  }
  /*
  PARAM:
      brightness: brightness recieved from app
  RETURN:
      int-> brightness converted to int
  parses recieved brightess
  */
  int parseBrightness(String brightness) {
    int mul = 1;
    int num = 0;
    int len = brightness.length();
    char brightArray[len];
    brightness.toCharArray(brightArray, len);
    for (int i = len - 1; i > -1; i--) {
      num = num + mul * (getValue(brightArray[i]));
      mul = mul * 10;
    }
    // Serial.print("converted : ");
    // Serial.println(num);
    return num;
  }
  /*
  PARAM:
      leds: pointer to the CRGB leds array
  RETURNS:
      void
  Party Mode implementation
  */
  void partyMode(CRGB *leds) {
    // delay(50);
    //Serial.println("generating colors");
    int r = random(30, 225);
    int g = random(30, 225);
    int b = random(30, 225);
    for (int i = 0; i < total_led; i++) {
      leds[i] = CRGB(r, g, b);
    }
    int bri = 105;
    int tmp = 50;
//    while (tmp <= bri) {
//      FastLED.setBrightness(tmp);
//      FastLED.show();
//      tmp += 40;
//      delay(100);
//    }
    // delay(500);
    r = random(30, 225);
    g = random(30, 225);
    b = random(30, 225);
    for (int i = 0; i < total_led; i++) {
      leds[i] = CRGB(r, g, b);
    }
//    while (bri >= 50) {
      FastLED.setBrightness(bri);
      Serial.println(bri);
      FastLED.show();
//      bri -= 40;
      delay(100);
//    }
  }
  /*
  PARAM:
      leds: pointer to CRGB leds array
  RETURN:
      void
  Gradient Mode implementation
  */
  void gradientMode(CRGB *leds) {
    // Serial.println("this mode ");
    for (int box = 0; box < num_of_boxes; box++) {
      for (int i = 0; i < 8; i++)
        leds[i + (box * 30)] = CRGB(primary[R], primary[G], primary[B]);
      for (int i = 8; i < 24; i++)
        leds[i + (box * 30)] = CRGB(secondary[R], secondary[G], secondary[B]);
      for (int i = 24; i < 30; i++)
        leds[i + (box * 30)] = CRGB(primary[R], primary[G], primary[B]);
      FastLED.setBrightness(brightness);
      FastLED.show();
      delay(80);
    }
  }
  /*
  PARAM:
      leds: pointer to CRGB leds array
  RETURN:
      void
  Twinkle Mode implementation
  */
  void twinkleMode(CRGB *leds) {
    uint16_t hsv_arr[3];
    uint16_t h = hsv_arr[0], s=hsv_arr[1], v=hsv_arr[2];
    rgb2hsv(primary[R], primary[G], primary[B],hsv_arr);
    for (int i = 0; i < total_led; i++)
      leds[i] = CHSV(h, s, v);
    
    FastLED.show();
    delay(1000);

    rgb2hsv(secondary[R], secondary[G], secondary[B],hsv_arr);

    int boxx = random(0, num_of_boxes);
    int n = 0;
    while (n < v) {
      for (int i = 0; i < 30; i++) {

        leds[i + (30 * boxx)] = CHSV(h,s, n);
      }
      n = n + 10;
      FastLED.show();
      delay(20);
    }
  
    delay(1000);
    rgb2hsv(primary[R], primary[G], primary[B],hsv_arr);
    int j = 0;

    while (j < v) {

      for (int i = 0; i < 30; i++) {
        leds[i + (30 * boxx)] = CHSV(h, s, j);
      }
      j = j + 10;
      FastLED.show();
      delay(20);
    }
    delay(1000);
  }
  /*
  PARAM:
      leds: pointer to CRGB array
  RETURN:
      void
  Diverse Mode Implementation
  */
  void diverseMode(CRGB *leds) {
    int trig = 0;
    char tmp[6];
    uint16_t len = diverse_color.length();
    char color[len];
    diverse_color.toCharArray(color, len);
    int box = 0;
    for (int i = 0; i < len; i++) {

      // Serial.print("trig: ");
      // Serial.println(trig);

      tmp[trig++] = color[i];

      if (trig == 6) {
        for (int i = 0; i < 30; i++) {
          leds[i + (box * 30)] =
              CRGB(value(tmp[0], tmp[1]), value(tmp[2], tmp[3]),
                   value(tmp[4], tmp[5]));
        }

        FastLED.show();
        delay(10);
        trig = 0;
        box++;
      }
    }
    delay(10);

//    for (int i = 30; i < 105; i++) {
//      FastLED.setBrightness(i);
//      FastLED.show();
//      delay(10);
//    }
//    delay(1000);
//    for (int i = 105; i > 30; i--) {
//      FastLED.setBrightness(i);
//      FastLED.show();
//      delay(10);
//    }
  }
  /*PARAM:
      leds: ptr to CRGB leds array
  RETURN:
      void
  Static and Night Mode implementation
  */
  void staticAndNightMode(CRGB *leds) {
    for (int i = 0; i < total_led; i++) {
      leds[i] = CRGB(primary[R], primary[G], primary[B]);
    }
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(80);
  }
};
