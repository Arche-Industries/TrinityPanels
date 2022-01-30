#include"Flakes.h"

#include <WiFi.h>

#define NUM_TOPICS 5

float fscale(float originalMin, float originalMax, float newBegin, float newEnd,
             float inputValue, float curve) {

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;

  // condition curve parameter
  // limit range

  if (curve > 10)
    curve = 10;
  if (curve < -10)
    curve = -10;

  curve =
      (curve * -.1); // - invert and scale - this seems more intuitive - postive
                     // numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for
                          // other pow function

  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin) {
    NewRange = newEnd - newBegin;
  } else {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal = zeroRefCurVal / OriginalRange; // normalize to 0 - 1 float

  // Check for originalMin > originalMax  - the math for all other cases i.e.
  // negative numbers seems to work out fine
  if (originalMin > originalMax) {
    return 0;
  }

  if (invFlag == 0) {
    rangedValue = (pow(normalizedCurVal, curve) * NewRange) + newBegin;
  } else // invert the ranges
  {
    rangedValue = newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}


void Flakes::init(Preferences *p, AsyncMqttClient *mcli, uint8_t num_panels,
                  Music_data *muPtr, WiFiUDP *udp, CRGB* crgb) {
  m_num_panels = num_panels;
  m_num_leds = m_num_panels * LEDS_PER_M;
  m_primary = CRGB::Red;
  m_secondary = CRGB::Green;
  m_pref = p;
  m_mqttClient = mcli;
  music = muPtr;
  m_UDP = udp;
  m_leds = crgb;
}
void Flakes::calibarate(int *arr) {
  float avg = 0;
  for (int i = 0; i < CALIBARATE_SAMPLE_SIZE; i++) {
    Serial.print("arr[]:  ");
    Serial.println(arr[i]);
    avg += arr[i];
  }
  avg /= 100;
  Serial.print("avg  ");
  Serial.println(avg);
  music->mic_low = (int)avg + 10;
  music->mic_high = (int)music->mic_low + 50;
  Serial.print("MIC_LOW  ");
  Serial.println(music->mic_low);
  delay(2000); // TODO remove delays and serial prints
}

void Flakes::setMode(Mode mode) {
  Flakes::mode = mode;
  // TODO: put mode in eeprom here.
}

void Flakes::setColor(CRGB color) {
  m_leds[0] = color;
  FastLED.show();
}

void Flakes::setCRGBPtr(CRGB *leds) {
  if (leds) {
    m_leds = leds;
  }
}
void Flakes::setNumPanels(uint8_t num) {
  m_num_panels = num;
  // TODO: remake CRGB array after this
  //      store m_num_panels in eeprom
}

void Flakes::setPrimary(uint8_t R, uint8_t G, uint8_t B) {
  m_primary.r = R;
  m_primary.g = G;
  m_primary.b = B;
}

void Flakes::setSecondary(uint8_t R, uint8_t G, uint8_t B) {
  m_secondary.r = R;
  m_secondary.g = G;
  m_secondary.b = B;
}

void Flakes::setDiverse(char *color, int len) {

  if (color == NULL || len == 0)
    return;
  char tmp[m_num_panels*6];
  for(int i=0; i<m_num_panels*6; i++){
    tmp[i] = color[i];
    Serial.printf("%d:%c|",i,tmp[i]);
  }
  String temp(tmp);
  m_diverse = "";
  m_diverse = temp;
  Serial.println("set m_diverse");
}

void Flakes::rgb2hsv(uint8_t r, uint8_t g, uint8_t b, uint16_t *hsv_arr) {
  float h, s, v;
  float cmin, cmax, delta;
  float rp, gp, bp;
  float maxVal = 255.0;

  rp = float(r / maxVal);
  gp = float(g / maxVal);
  bp = float(b / maxVal);
  cmax = rp > gp ? (rp > bp ? rp : bp) : (gp > bp ? gp : bp);
  cmin = rp < gp ? (rp < bp ? rp : bp) : (gp < bp ? gp : bp);
  delta = cmax - cmin;
  s = cmax == 0 ? 0 : (delta / cmax) * 100;
  v = cmax * 100;
  h = delta == 0   ? 0
      : cmax == rp ? (int((gp - bp) / delta) % 6)
      : cmax == gp ? (((bp - rp) / delta) + 2)
                   : (((rp - gp) / delta) + 4);
  h = h * 60;
  if (h < 0) {
    h = 360 + h;
  }
  hsv_arr[0] = (uint16_t)(h)*0.708;
  hsv_arr[1] = (uint16_t)(s)*2.55;
  hsv_arr[2] = (uint16_t)(v)*2.55;
}

 int Flakes::hexToInt(char a) {
  if (a < 97) {
    // for 0-9
    int res =  (int)a - 48;
    return res;
  } else {
    // for a-f
    return (int)a - 87;
  }
}

int Flakes::value(char a, char b) { return (16 * (hexToInt(a)) + hexToInt(b)); }

void Flakes::connectMQTT( const char *HOST, int PORT) {
  m_mqttClient->connect();
}

void Flakes::disconnectMQTT(bool force = false) {
  m_mqttClient->disconnect(force);
}

void Flakes::allBlack() {
  if (m_leds == NULL)
    return;
  for (int i = 0; i < m_num_leds; i++) {
    m_leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void Flakes::indicateMode() {
  // Mode change
  allBlack();
  m_leds[0] = CRGB::Blue;
  FastLED.show();
  delay(500);
  m_leds[0] = CRGB::Black;
  FastLED.show();
}

void Flakes::indicateConnect() {
  allBlack();
  m_leds[0] = CRGB::Red;
  FastLED.show();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  m_leds[0] = CRGB::Green;
  FastLED.show();
  delay(1000);
  allBlack();
}

void Flakes::indicateDisconnect() {
  allBlack();
  m_leds[0] = CRGB::Yellow;
  FastLED.show();
  delay(1000);
  allBlack();
}

void Flakes::indicateBoot() {
  allBlack();
  m_leds[0] = CRGB::White;
  FastLED.show();
  delay(1000);
  allBlack();
}

void Flakes::checkColor() {
  Serial.println("setting red");
  for (int i = 0; i < m_num_leds; i++) {
    //Serial.println(i);
    m_leds[i] = CRGB::Green;
    delay(1);
  }
  FastLED.show();
  Serial.println("Setting black");
  delay(100);
  for (int i = 0; i < m_num_leds; i++) {
    //Serial.println(i);
    m_leds[i] = CRGB(0, 0, 0);
    delay(1);
  }
  FastLED.show();
}

void Flakes::savePreset() {
  m_pref->putUChar("mode", mode); // current mode in eeprom
  m_pref->putUChar("panels", m_num_panels);
  uint8_t primary[3] = {m_primary[0], m_primary[1], m_primary[2]};
  m_pref->putBytes("pcolor", primary, 3);
  uint8_t secondary[3] = {m_secondary[0], m_secondary[1], m_secondary[2]};
  m_pref->putBytes("scolor", secondary, 3);
}
void Flakes::loadPreset() {
  mode = (Mode)m_pref->getUChar("mode", 0);
  m_num_panels = m_pref->getUChar("panels", 6);
  uint8_t pbuf[3], sbuf[3];
  m_pref->getBytes("pcolor", pbuf, 3);
  m_pref->getBytes("scolor", sbuf, 3);
  int i = 3;
  while (i--) {
    m_primary[i - 1] = pbuf[i - 1];
    m_secondary[i - 1] = sbuf[i - 1];
  }
}
void Flakes::staticMode(){
    for(int i=0; i<m_num_leds; i++){
        m_leds[i] = m_primary;
    }
    FastLED.setBrightness(70);
    FastLED.show();
    delay(30);
}
void Flakes::gradientMode() {
  for (int box = 0; box < m_num_panels; box++) {
    if (box % 2 != 0) {
      for (int i = 0; i < 8; i++)
        m_leds[i + (box * 30)] = m_primary;
      for (int i = 8; i < 30; i++)
        m_leds[i + (box * 30)] = m_secondary;
    } else {
      for (int i = 0; i < 19; i++)
        m_leds[i + (box * 30)] = m_primary;
      for (int i = 20; i < 30; i++)
        m_leds[i + (30 * box)] = m_secondary;
    }
  }
  FastLED.setBrightness(70);
  FastLED.show();
  delay(30);
}

void Flakes::twinkleMode() {
  uint16_t hsv_arr[3];
  uint16_t primArr[3];
  rgb2hsv(m_primary[0], m_primary[1], m_primary[2],
          hsv_arr);
  uint16_t h = hsv_arr[0], s = hsv_arr[1], v = hsv_arr[2];
  primArr[0] = h;
  primArr[1] = s;
  primArr[2] = v;
  for (int i = 0; i < m_num_leds; i++)
    m_leds[i] = CHSV(h, s, v);
  FastLED.show();

  rgb2hsv(m_secondary[0], m_secondary[1],
          m_secondary[2], hsv_arr);
  h = hsv_arr[0], s = hsv_arr[1], v = hsv_arr[2];
  int boxx = random(0, m_num_panels);
  int n = 0;
  while (n < v) {
    for (int i = 0; i < 30; i++) {
      m_leds[i + (30 * boxx)] = CHSV(h, s, n);
    }
    n = n + 10;
    FastLED.show();
    delay(20);
  }
  delay(1000);
  h = primArr[0], s = primArr[1], v = primArr[2];
  int j = 0;
  while (j < v) {
    for (int i = 0; i < 30; i++) {
      m_leds[i + (30 * boxx)] = CHSV(h, s, j);
    }
    j = j + 10;
    FastLED.show();
    delay(20);
  }
}
void Flakes::diverseMode() {
  // TODO: populate m_diverse String
  Serial.println("setting color");
  int trig = 0;
  char tmp[6];
  const uint16_t len = m_num_panels*6;
  char color[len];
  m_diverse.toCharArray(color, len+1);
  int box = 0;
  for (int i = 0; i < len; i++) {
    Serial.printf("%c/",color[i]);
    tmp[trig++] = color[i];
    if (trig == 6) {
      for (int i = 0; i < 30; i++) {
        m_leds[i + (box * 30)] =
            CRGB(value(tmp[0], tmp[1]), value(tmp[2], tmp[3]),
                 value(tmp[4], tmp[5]));
      }
      FastLED.show();
      delay(10);
      trig = 0;
      box++;
    }
    if (box > m_num_panels)
      return;
  }
  delay(10);
  Serial.println("diverseMode() end");
}
void Flakes::musicMode() {
  int sanityValue = music->sanityBuffer->computeAverage();
  if (!(abs(analogRaw - sanityValue) > BUFFER_DEVIATION)) {
    music->sanityBuffer->setSample(analogRaw);
  }
  analogRaw = fscale(music->mic_low, music->mic_high, music->mic_low,
                     music->mic_high, analogRaw, 0.4);
  if (music->samples->setSample(analogRaw))
    return;
  uint16_t longTermAverage = music->longTermSamples->computeAverage();
  uint16_t useVal = music->samples->computeAverage();
  music->longTermSamples->setSample(useVal);

  int diff = (useVal - longTermAverage);
  if (diff > 5) {
    if (music->globalHue < 235) {
      music->globalHue += music->hueIncrement;
    }
  } else if (diff < -5) {
    if (music->globalHue < 2) {
      music->globalHue -= music->hueIncrement;
    }
  }

  int curshow = fscale(music->mic_low, music->mic_high, 0.0, (float)LEDS_PER_M,
                       (float)useVal, 0);
  if (curshow > 15)
    curshow = 15;

  for (int i = 0; i < LEDS_PER_M / 2; i++) {
    for (int box = 0; box < Flakes::m_num_panels; box++) {
      music->baseStart = music->ledAddr[box];
      music->start = box * 30;
      music->finish = music->start + 30;
      music->fwAddr = music->baseStart + i;
      music->revAddr = music->baseStart - i;
      if (music->fwAddr >= music->finish) {
        music->fwAddr = music->start + (music->fwAddr % music->finish);
      }
      if (i < curshow) {
        m_leds[music->fwAddr] =
            CHSV(music->globalHue + music->hueOffset + (i * 2), 255, 255);
        m_leds[music->revAddr] =
            CHSV(music->globalHue + music->hueOffset + (i * 2), 255, 255);
      } else {
        m_leds[music->fwAddr] =
            CRGB(m_leds[i].r / music->fadeScale, m_leds[i].g / music->fadeScale,
                 m_leds[i].b / music->fadeScale);
        m_leds[music->revAddr] =
            CRGB(m_leds[music->revAddr].r / music->fadeScale,
                 m_leds[music->revAddr].g / music->fadeScale,
                 m_leds[music->revAddr].b / music->fadeScale);
      }
    }
    //TODO: delays?
  }
}
