struct ADCRead {
  ADCRead(int p) : pin(p) {}

  void setup() {
    pinMode(pin, ANALOG);
    analogReadResolution(10);
  }

  float getIfChanged() {
    auto now = millis();
    auto raw = analogRead(pin);
    float curRaw = 1.f - raw * 1.f / 1024;
    cur = curRaw * alpha + cur * (1.f - alpha);
    if (now - lastTimeCheck < minResMs)
      return -1;
    lastTimeCheck = now;
    if (abs(lastSent - cur) >= epsilon) {
      cur = int(cur * 1.f / epsilon) * epsilon;
      if (cur <= epsilon)
        cur = 0;
      else if (cur >= 1 - epsilon)
        cur = 1;
      Serial.println(raw);

      lastSent = cur;
      return cur;
    }
    return -1;
  }

  float epsilon = 0.009;
  int minResMs = 5;
  float lastSent = -1;
  float cur = 0;
  float alpha = 0.1;
  int pin;
  unsigned long lastTimeCheck = 0;
};
