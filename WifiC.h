#include "esp_private/wifi.h"
#include <WiFi.h>
#include <WiFiMulti.h>
WiFiMulti multi;

#include <Preferences.h>
#include <nvs_flash.h>

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

bool tryConnectToWifi() {

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

    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    setLed(0, RGB_BRIGHTNESS, 0);

    delay(1000);
    Serial.print("Successfully connected to network: ");
    Serial.println(WiFi.SSID());

    Serial.print(":: ");
    Serial.println(int(WiFi.getTxPower()));
    Serial.print(":: ");
    Serial.println(WiFi.localIP().toString());
    return true;
    // esp_wifi_config_80211_tx_rate((wifi_interface_t)WIFI_IF_STA,
    //                               WIFI_PHY_RATE_MCS5_LGI);
    // to enable fixed rate setting it to 802.11n, modulation coding scheme 3
    // you can use:
    // ESP_ERROR_CHECK(esp_wifi_internal_set_fix_rate(WIFI_IF_STA, true,
    //                                                WIFI_PHY_RATE_MCS3_SGI));
  } else {
    setLed(RGB_BRIGHTNESS, 0, 0);
    Serial.println("Failed to connect to a WiFi network");
    delay(100);
#if 1
    printNets();
#endif
    WiFi.disconnect(true, true);

    delay(10000);
  }
  return false;
}
