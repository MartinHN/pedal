#include <Arduino.h>
#define RGB_BRIGHTNESS 10 // Change white brightness (max 255)
#define RGB_BUILTIN 21

#include "OTAUpdater.h"
OTAUpdater myOTA;

//////////
// conf
const char *piAddress = "10.42.0.1";
const char *multiCastAddress = "230.1.1.1";
// const char *udpAddress = piAddress;
const char *udpAddress = multiCastAddress;
const int udpRemotePort = 9000;
const int udpLocalPort = 4444;

//////////////////
// forward
void setLed(uint8_t r, uint8_t g, uint8_t b);

///////

#include "ADCRead.h"
#include "OSCHelpers.h"
ADCRead adc(1);
#include "WifiC.h"
void setup() {

  // USB CDC doesn't need a baud rate
  Serial.begin();
  delay(1000);
  setLed(0, 0, RGB_BRIGHTNESS);
  connectToWifiOrDie();
  setupOSC();
  myOTA.setup("pedal");
  myOTA.begin();

  adc.setup();
  Serial.println("\r\nStarting...\r\n");
}

uint32_t fc = 0;
bool hasShutdownLed = false;

void setLed(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_BUILTIN, g, r, b);
}

void setPedalValue(float p) {
  auto mapV = minPedal + p * (maxPedal - minPedal);
  if (!hasShutdownLed) {
    hasShutdownLed = true;
    int v = 0; // p * RGB_BRIGHTNESS;
    // Serial.print("val");Serial.println(v);
    setLed(v, v, v);
  }
  // int v = p * RGB_BRIGHTNESS;
  // setLed(v, v, v);
  sendMsg(mapV);
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
  if (newVal >= 0) {
    setPedalValue(newVal);
  }
  OSCMessage msg = {};
  if (receiveOSC(msg)) {
    // do smthing
  } else {
    delayMicroseconds(100);
  }
#else
  int periodMs = 10;
  int rampUp = 2000;
  int total = rampUp / periodMs;
  setPedalValue((fc % total) * 1.f / total);
  delay(periodMs);
#endif

  fc++;
}
