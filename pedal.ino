#include <Arduino.h>
#define RGB_BRIGHTNESS 10 // Change white brightness (max 255)
#define RGB_BUILTIN 21

#include "OTAUpdater.h"
OTAUpdater myOTA;

#include <vector>
//////////
// conf
const char *piAddressW = "10.0.0.10";
const char *piAddress = "10.0.0.101";
const char *pi2AddressW = "10.0.0.11";
// const char *pi2Address = "10.0.0.11";
// const char *multicastAddr = "230.1.1.1";
// const char *smartAddress = "10.0.0.140";
const char *smartAddress = "192.168.138.34";

std::vector<IPAddress> pi1UdpAddresses;
std::vector<IPAddress> pi2UdpAddresses;

// const char *udpAddresses = multiCastAddress;
const int udpRemotePort = 9000;
const int udpLocalPort = 4444;

//////////////////
// forward
void setLed(uint8_t r, uint8_t g, uint8_t b);
void fakeShutdown();

///////

#include "ADCRead.h"
#include "Button.h"
#include "OSCHelpers.h"
ADCRead adc(1);

Button b0(8);
Button b1(7);
#include "WifiC.h"
void setup() {

  // USB CDC doesn't need a baud rate
  Serial.begin();
  delay(1000);
  setLed(0, 0, RGB_BRIGHTNESS);
  IPAddress ip;
  ip.fromString(piAddressW);
  pi1UdpAddresses.push_back(ip);
  ip.fromString(piAddress);
  pi1UdpAddresses.push_back(ip);
  ip.fromString(pi2AddressW);
  pi2UdpAddresses.push_back(ip);
  // ip.fromString(multicastAddr);
  // udpAddresses.push_back(ip);
  ip.fromString(smartAddress);
  pi1UdpAddresses.push_back(ip);

  if (!tryConnectToWifi()) {
    preferences.begin("pedal", true);
    // Note: Key name is limited to 15 chars.

    unsigned int failedBoot = preferences.getUInt("failedBoot", 0);
    failedBoot++;
    preferences.putUInt("failedBoot", failedBoot);
    preferences.end();
    if (failedBoot > 200)
      fakeShutdown();
    else
      ESP.restart();
  }
  setupOSC();
  myOTA.setup("pedal");
  myOTA.begin();

  adc.setup();
  b0.setup();
  b1.setup();
  Serial.println("\r\nStarting...\r\n");
}

uint32_t fc = 0;
bool hasShutdownLed = false;
bool lastMessageSent = true;

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
  lastMessageSent = sendFloatMsg(mapV);
  if (!lastMessageSent) {
    Serial.println("no message sent");
  }
  Serial.println("sentPedal : " + String(p, 4));
}

void setButtonValue(int bnum, bool val) { sendBtnMsg(bnum, val); }

unsigned long lastPedalUpdate = 0;
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("disconnected from  a WiFi network");
    delay(5000);
    ESP.restart();
  }
  myOTA.handle();
#if 1
  auto newVal = adc.getIfChanged();
  auto now = millis();
  if (newVal >= 0) {
    // Serial.println(now - lastPedalUpdate);
    lastPedalUpdate = now;
    setPedalValue(newVal);
  }

  auto b0Val = b0.getIfChanged();
  if (b0Val >= 0) {
    lastPedalUpdate = now;
    setButtonValue(0, b0Val);
  }
  auto b1Val = b1.getIfChanged();
  if (b1Val >= 0) {
    lastPedalUpdate = now;
    setButtonValue(1, b1Val);
  }
  // if not moved since more than 3 hours
  if (now - lastPedalUpdate > 3 * 60 * 60 * 1000) {

    Serial.println("no activity going to sleep");
    fakeShutdown();
  }
  OSCMessage msg = {};
  if (receiveOSC(msg)) {
    // do smthing
  } else {
    // delayMicroseconds(100);
  }
  delay(10);
#else
  int periodMs = 10;
  int rampUp = 2000;
  int total = rampUp / periodMs;
  setPedalValue((fc % total) * 1.f / total);
  delay(periodMs);
#endif

  fc++;
}

void fakeShutdown() {
  preferences.begin("pedal", false);
  preferences.putUInt("failedBoot", 0);
  preferences.end();
  // kill wifi if exists
  WiFi.mode(WIFI_MODE_NULL);
  esp_deep_sleep_start();
}
