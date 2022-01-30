#include <AsyncMqttClient.h>
#include <FastLED.h>
#include <Preferences.h>
#include <WiFiUdp.h>
#define LED_PIN 4
#define LEDS_PER_M 30
#define LEDS_DEFAULT 180
//#define MQTT_HOST "192.168.1.4"
#define MQTT_HOST "app.archeindustries.com"
#define MQTT_PORT 1883
#define SAMPLE_SIZE 15
#define LONG_TERM_SAMPLES 250
#define BUFFER_DEVIATION 400
#define BUFFER_SIZE 3
#define CALIBARATE_SAMPLE_SIZE 100

typedef unsigned char uint8_t;

enum Mode { STATIC, TWINKLE, GRADIENT, DIVERSE, MUSIC };

struct led_command {
  uint8_t opmode;
  uint32_t data;
};
typedef struct {
  uint32_t mic_low = 10;
  uint32_t mic_high = 60;
  float globalHue = 0;
  float globalBrightness = 255;
  int hueOffset = 120;
  float fadeScale = 1.3;
  float hueIncrement = 0.9;
  bool calibarated = false;
  int cal_arr[CALIBARATE_SAMPLE_SIZE];
  int cal_cnt = 0;
  struct averageCounter *samples;
  struct averageCounter *longTermSamples;
  struct averageCounter *sanityBuffer;
  int ledAddr[50];
  int start;
  int finish;
  int fwAddr;
  int revAddr;
  int baseStart;
  led_command cmd;
  bool lastActive = false;
} Music_data;

class Flakes {
public:
  Music_data *music;
  Mode mode;
  int analogRaw;
  uint8_t m_num_panels;
  uint8_t m_num_leds;
  CRGB m_primary;
  CRGB m_secondary;
  String m_diverse;
  Preferences *m_pref;
  AsyncMqttClient *m_mqttClient;
  WiFiUDP *m_UDP;
  CRGB *m_leds;

  void init(Preferences *, AsyncMqttClient *, uint8_t, Music_data *, WiFiUDP *,CRGB*);
  void calibarate(int *arr);
  void setMode(Mode mode);
  void setColor(CRGB);
  void setCRGBPtr(CRGB *leds);
  void setNumPanels(uint8_t);
  void setPrimary(uint8_t, uint8_t, uint8_t);
  void setSecondary(uint8_t, uint8_t, uint8_t);
  void setDiverse(char *, int);
  void rgb2hsv(uint8_t r, uint8_t g, uint8_t b, uint16_t *hsv_arr);
  int  hexToInt(char a);
  int  value(char a, char b);
  void connectMQTT(const char *HOST, int PORT);
  void disconnectMQTT(bool);
  void allBlack();
  void indicateMode();
  void indicateConnect();
  void indicateDisconnect();
  void indicateBoot();
  void checkColor();
  void savePreset(); // TODO: when to call these
  void loadPreset();
  void staticMode();
  void gradientMode();
  void twinkleMode();
  void diverseMode();
  void musicMode();
};


typedef struct averageCounter {
  uint16_t *samples;
  uint16_t sample_size;
  uint8_t counter;

  averageCounter(uint16_t size) {
    counter = 0;
    sample_size = size;
    samples = (uint16_t *)malloc(sizeof(uint16_t) * sample_size);
  }

  bool setSample(uint16_t val) {
    if (counter < sample_size) {
      samples[counter++] = val;
      return true;
    } else {
      counter = 0;
      return false;
    }
  }

  int computeAverage() {
    int accumulator = 0;
    for (int i = 0; i < sample_size; i++) {
      accumulator += samples[i];
    }
    return (int)(accumulator / sample_size);
  }
} averageCounter;
