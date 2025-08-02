#define USE_ADC_TASK 1
void readPinTask(void *ADCReadPtr);

struct ADCRead {
  ADCRead(int p) : pin(p) {}
  static constexpr int analogRes = 12;
  static constexpr int numReadings = 8;
  float readings[numReadings]; // the readings from the analog input
  int readIndex = 0;           // the index of the current reading
  float total = 0;             // the running total

  void setup() {
    pinMode(pin, ANALOG);
    analogReadResolution(analogRes);
    for (int i = 0; i < numReadings; i++) {
      readings[i] = 0;
    }
#if USE_ADC_TASK
    xTaskCreatePinnedToCore(readPinTask, "ADC Task", 2048, this, 0, NULL, 0);
#endif
  }

  float getIfChanged() {
    auto now = millis();
#if !USE_ADC_TASK
    cur = updateAVG();
#endif

    // cur = curRaw * alpha + cur * (1.f - alpha);
    if (now - lastTimeCheck < minResMs)
      return -1;
    lastTimeCheck = now;
    if (cur <= epsilon)
      cur = 0;
    else if (cur >= 1 - epsilon)
      cur = 1;
    if (abs(lastSent - cur) >= 2 * epsilon || (cur == 0 && lastSent > 0) ||
        (cur == 1 && lastSent < 1)) {
      if (cur > 0 && cur < 1)
        cur = std::roundf(cur * 1.f / epsilon) * epsilon;

      //   Serial.println(raw);

      lastSent = cur;
      return cur;
    }
    return -1;
  }

  float updateAVG() {
    auto raw = float(analogRead(pin));
    float curRaw = 1.f - raw * 1.f / std::pow(2, analogRes);

    total = total - readings[readIndex];
    readings[readIndex] = curRaw;
    total = total + readings[readIndex];
    readIndex = readIndex + 1;
    if (readIndex >= numReadings) {
      // ...wrap around to the beginning:
      readIndex = 0;
    }
    float average = total * 1.f / numReadings;
    auto stepAvg = std::roundf(average * 1.f / epsilon) * epsilon;
    return stepAvg;
  }
  float epsilon = 0.001;
  int minResMs = 15;
  float lastSent = -1;
  float cur = 0;
  // float alpha = 0.1;
  int pin;
  unsigned long lastTimeCheck = 0;
};

void readPinTask(void *ADCReadPtr) {
  auto *ptr = static_cast<ADCRead *>(ADCReadPtr);
  while (true) {
    ptr->cur = ptr->updateAVG();
    static_assert(1 / portTICK_PERIOD_MS > 0);
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
