
#define DEBOUNCEBTN 0
struct Button {
  Button(uint8_t p) : pin(p) {}

  static constexpr int minResMs = 1;
#if DEBOUNCEBTN
  static constexpr int numReadings = 35;
  bool readings[numReadings]; // the readings from the digital input
  int readIndex = 0;          // the index of the current reading
#endif
  void setup() {
    pinMode(pin, INPUT_PULLUP);
#if DEBOUNCEBTN
    for (int i = 0; i < numReadings; i++)
      readings[i] = 0;
#endif
  }

  int getIfChanged() {
    auto now = millis();
    if (now - lastTimeCheck < minResMs)
      return -1;

    lastTimeCheck = now;
    bool curVal = digitalRead(pin) == HIGH;
#if DEBOUNCEBTN // no deboun
    readings[readIndex] = curVal;
    readIndex = (readIndex + 1) % numReadings;

    int numOn = 0;
    for (int i = 0; i < numReadings; i++) {
      if (readings[i])
        numOn++;
    }
    bool curState = numOn > numReadings / 2;
#else
    bool curState = curVal;
#endif
    if (curState == lastState)
      return -1;
    lastState = curState;
    return curState ? 1 : 0;
  }

  uint8_t pin;
  unsigned long lastTimeCheck = 0;
  bool lastState = false;
};
