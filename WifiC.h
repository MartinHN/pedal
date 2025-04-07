#include <WiFi.h>
#include <WiFiMulti.h>
WiFiMulti multi;


#include <nvs_flash.h>
#include <Preferences.h>

Preferences preferences;

bool tryConnect() {
#if 1
  return (multi.run() == WL_CONNECTED);
#else
  static bool hasAsked = false;
  if (!hasAsked) {
    hasAsked = true;
    WiFi.begin("booyakachar-sl", "booyakachar");
  }
  return WiFi.status() == WL_CONNECTED;
#endif
}

void printNets() {
  WiFi.scanDelete();
  auto scanResult = WiFi.scanNetworks();
  if (scanResult < 0) {
    Serial.println("cant scan");
    return;
  }
  for (size_t i = 0; i < scanResult; i++)
    Serial.println(String(WiFi.SSID(i)) + ": " + WiFi.RSSI(i));
}

void connectToWifiOrDie() {

  {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
  }

  preferences.begin("pedal", true);
  // Note: Key name is limited to 15 chars.
  unsigned int failedBoot = preferences.getUInt("failedBoot", 0);
  preferences.end();
  if (failedBoot > 200){
    // kill wifi if exists
    WiFi.mode(WIFI_MODE_NULL);
    esp_deep_sleep_start();
  }

#include "secrets.hpp"
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setMinSecurity(WIFI_AUTH_WPA_PSK);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.disconnect(false, true);

  auto endTime = millis() + 15 * 1000;
  while ((millis() < endTime) && !tryConnect()) {
    // Serial.print("status");
    Serial.print(WiFi.status());
    // Serial.print(":: ");
    // Serial.println(int(WiFi.getTxPower()));
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    setLed(0, RGB_BRIGHTNESS, 0);
    delay(1000);
    Serial.print("Successfully connected to network: ");
    Serial.println(WiFi.SSID());
  } else {
    setLed(RGB_BRIGHTNESS, 0, 0);
    Serial.println("Failed to connect to a WiFi network");
    delay(100);
#if 1
    printNets();
#endif
    WiFi.disconnect(true, true);
    preferences.begin("pedal", false);
    failedBoot++;
    preferences.putUInt("failedBoot", failedBoot);
    preferences.end();
    delay(10000);

    ESP.restart();
  }
}
