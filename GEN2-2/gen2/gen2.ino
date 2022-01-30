#include "Flakes.h"
#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiManager.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
const char* TOPIC_RESET_CREDS = "FL01021745231UMxruu225/reset";
const char* TOPIC_MODE = "FL01021745231UMxruu225/mode";
const char* TOPIC_PCOLOR = "FL01021745231UMxruu225/pcolor";
const char* TOPIC_SCOLOR = "FL01021745231UMxruu225/scolor";
const char* TOPIC_ONLINE =  "FL01021745231UMxruu225/online";
const char* TOPIC_NUM_PANELS = "FL01021745231UMxruu225/nop";

WiFiManager wm;
WiFiUDP UDP; // can make this dynamically when mode is MUSIC
Flakes panel;
CRGB *leds;
Preferences pref;
AsyncMqttClient mqttClient;
Music_data music;

void(Flakes::*ptrMode[5])(void);

void processAddr(int *ledAddr) {
  ledAddr[0] = 0;
  int start = 0;
  int sudoStart = 0;
  int ADDR_SIZE = 50;

  ledAddr[0] = 30;
  ledAddr[1] = 55;
  ledAddr[2] = 80;
  for (int i = 3; i < ADDR_SIZE; i++) {
    if (i % 2 == 0) {
      ledAddr[i] = ledAddr[i - 1] + 25;
    } else {
      ledAddr[i] = ledAddr[i - 1] + 35;
    }
  }

  Serial.println("ADDR ARRAY-->");
  for (int i = 0; i < ADDR_SIZE; i++) {
    Serial.println(ledAddr[i]);
  }
  delay(2000); // todo remove delays and serial loggings
}
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pref.begin("FLAKES");
  uint8_t num_panel = pref.getUChar("panels", 6);
  Serial.printf("Number of panels in eeprom:%d ", num_panel);
  leds = new CRGB[num_panel * LEDS_PER_M];
 
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, num_panel * LEDS_PER_M);
  panel.init(&pref, &mqttClient, num_panel, &music, &UDP,leds);
  bool res;
  // TODO: indicate connecting to wifi
  Serial.println("added leds, connecting to wifi");
  panel.setColor(CRGB::Red);
  res = wm.autoConnect("FL-Trinity");
  if (!res) {
    Serial.println("Failed to connect to WiFi");
  } else {
    Serial.println("Connected to WiFi");
    panel.setColor(CRGB::Green);
    // TODO: ping for internet here.
  }

  /*Presuming getting internet(do a check). connect to MQTT */

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  panel.connectMQTT(MQTT_HOST, MQTT_PORT);
  processAddr(music.ledAddr);
  panel.checkColor();
  ptrMode[0] = &Flakes::staticMode;
  ptrMode[1] = &Flakes::twinkleMode;
  ptrMode[2] = &Flakes::gradientMode;
  ptrMode[3] = &Flakes::diverseMode;
  ptrMode[4] = &Flakes::musicMode;
}
void loop() {
//  Serial.println("looping");
  if (panel.mode == TWINKLE) {
    panel.twinkleMode();
  } else if (panel.mode == MUSIC) {
    int packetSize = UDP.parsePacket();
    if (packetSize)
      UDP.read((char *)&(music.cmd), sizeof(led_command));
    int opMode = music.cmd.opmode;
    panel.analogRaw = music.cmd.data;
    if (!music.calibarated) {
      if (music.cal_cnt < CALIBARATE_SAMPLE_SIZE) {
        if (panel.analogRaw != 0)
          music.cal_arr[music.cal_cnt++] = panel.analogRaw;
      } else {
        panel.calibarate(music.cal_arr);
        music.calibarated = true;
      }
    }
    panel.musicMode();
  }
  delay(50);
}
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT");
  // uint16_t packetIdArr[NUM_TOPICS];

  /*Subscribing to all the topics*/
  mqttClient.subscribe(TOPIC_RESET_CREDS, 0);
  mqttClient.subscribe(TOPIC_PCOLOR, 0);
  mqttClient.subscribe(TOPIC_SCOLOR, 0);
  mqttClient.subscribe(TOPIC_MODE, 0);
  mqttClient.subscribe(TOPIC_NUM_PANELS, 2);

  /*Publishing online*/
  mqttClient.publish(TOPIC_ONLINE, 0, true, "1");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT");
  panel.setColor(CRGB::Red);
  if (!WiFi.isConnected()) {
    Serial.println("WiFi not found. Restarting");
    ESP.restart();
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.printf("Subscribed: %d", packetId);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.printf("Unsubscribed: %d", packetId);
}

void onMqttMessage(char *topic, char *payload,
                           AsyncMqttClientMessageProperties properties,
                           size_t len, size_t index, size_t total) {
  Serial.printf("TOPIC:%s",topic);
  Serial.printf("PAYLOAD: %s\n",payload);
  String topicStr(topic);
  Serial.println(topicStr);
  if (topicStr == TOPIC_MODE) {
    Serial.println("got mode");
    panel.mode = (Mode)panel.value('0', payload[0]);
    Serial.printf("Mode--->%d\n",panel.mode);
    if(panel.mode > 5)
      return;
    if (panel.mode == MUSIC) {
      UDP.begin(7001);
      panel.music->samples = new averageCounter(SAMPLE_SIZE);
      panel.music->longTermSamples = new averageCounter(LONG_TERM_SAMPLES);
      panel.music->sanityBuffer = new averageCounter(BUFFER_SIZE);
      while (panel.music->sanityBuffer->setSample(250) == true) {
      }
      while (panel.music->longTermSamples->setSample(200) == true) {
      }
      panel.music->lastActive = true;
    }
    if (panel.music->lastActive) {
      UDP.stop();
      delete (panel.music->samples);
      delete (panel.music->longTermSamples);
      delete (panel.music->sanityBuffer);
      panel.music->lastActive = false;
      
    }

  } else if (topicStr == TOPIC_PCOLOR) {
    Serial.println("got pcolor");
    /* Setting primary color and calling mode*/
    if(panel.mode == DIVERSE){
      Serial.println("diverse mode: setting color");
      panel.setDiverse(payload,len);
    } 
    else if (panel.mode == TWINKLE) // for twinkle mode we call it continuously in loop()
        return;

    else{
        panel.m_primary = CRGB(panel.value(payload[0], payload[1]), panel.value(payload[2], payload[3]),panel.value(payload[4], payload[5]));
      }
    
    (panel.*ptrMode[panel.mode])();
    
  } else if (topicStr == TOPIC_SCOLOR) {
    Serial.println("got scolor");
    /*Setting secondary color and calling mode*/
    panel.m_secondary =
        CRGB(panel.value(payload[0], payload[1]), panel.value(payload[2], payload[3]),
             panel.value(payload[4], payload[5]));
    if (panel.mode == TWINKLE)
      return;
    (panel.*ptrMode[panel.mode])();
  } else if (topic == TOPIC_RESET_CREDS) {
    if (payload == "1") {
      /*TODO: reset wifi creds and restart*/
    }
  } else if (topicStr == TOPIC_NUM_PANELS) {
    panel.checkColor();
    panel.m_num_panels = panel.value(payload[0], payload[1]);
    panel.m_num_leds = LEDS_PER_M * panel.m_num_panels;
    Serial.printf("num of panels-> %d\n",panel.m_num_panels);
    pref.putUChar("panels", panel.m_num_panels);
    ESP.restart(); //cant dealloc already added leds so restarting to run FastLED.addLEDS<>() again.
  }
  else{
    Serial.println("Invalid topic");
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Pub ack");
  Serial.print("packet id:  ");
  Serial.println(packetId);
}
