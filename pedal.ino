#include <Arduino.h>
#define RGB_BRIGHTNESS 20 // Change white brightness (max 255)
#define RGB_BUILTIN 21

#include <WiFi.h>
#include "OTAUpdater.h"
OTAUpdater myOTA;
#include <WiFiMulti.h>
#include <WiFiUdp.h>
WiFiUDP udp;
#include <OSCMessage.h>

#include "ADCRead.h"

const char *udpAddress = "230.1.1.1";
const int udpPort = 9000;

ADCRead adc(1);
void setup() {

  // USB CDC doesn't need a baud rate
  Serial.begin();
  delay(1000);
  setLed(0, 0, 100);
#if 0
    printNets();
#endif
  WiFiMulti multi;
#include "secrets.hpp"
  if (multi.run(10000) == WL_CONNECTED) {
    setLed(0, 100, 0);
    delay(1000);
    Serial.print("Successfully connected to network: ");
    Serial.println(WiFi.SSID());
  } else {
    setLed(0, 100, 0);
    Serial.println("Failed to connect to a WiFi network");
    delay(5000);
    ESP.restart();
  }

  myOTA.setup("pedal");
  myOTA.begin();

  adc.setup();
  Serial.println("\r\nStarting...\r\n");
}

int fc = 0;
int periodMs = 10;

void setLed(int r, int g, int b) { neopixelWrite(RGB_BUILTIN, r, b, g); }
void setPedalValue(float p) {
  int v = p * RGB_BRIGHTNESS;
  // Serial.print("val");Serial.println(v);
  setLed(v, v, v);
  sendMsg(p);
}

void sendMsg(float v) {

  // Send a packet
  //  udp.beginPacket(udpAddress,udpPort);
  //  udp.printf("Seconds since boot: %lu", millis()/1000);
  //  udp.endPacket();

  OSCMessage msg("/obsidian/session");
  msg.add("macro4");
  msg.add(v);
  udp.beginPacket(udpAddress, udpPort);
  msg.send(udp);
  udp.endPacket();
}
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("disconnected from  a WiFi network");
    delay(5000);
    ESP.restart();
  }
  myOTA.handle();
#if 1
  auto newVal = adc.getIfChanged();
  if (newVal >= 0)
  {
    setPedalValue(newVal);
  }
delay(1);
#else
  int rampUp = 2000;
  int total = rampUp / periodMs;
  setPedalValue((fc % total) * 1.f / total);
  delay(periodMs);
  #endif

  fc++;
}

void printNets() {
  auto scanResult = WiFi.scanNetworks();
  if (scanResult < 0)
    Serial.println("cant scan");

  for (size_t i = 0; i < scanResult; i++)
    Serial.println(String(WiFi.SSID(i)) + ": " + WiFi.RSSI(i));
}
