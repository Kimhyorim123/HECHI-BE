#ifndef HARDWARE_H
#define HARDWARE_H

class Button {
private:
  unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 30;

  bool lastReading;
  bool debouncedState;
  bool lastPressed = false;

  int pin;

  void refresh() {
    bool reading = digitalRead(pin);
    unsigned long now = millis();

    if (reading != lastReading)
      lastDebounceTime = now;
    
    if ((now - lastDebounceTime) > debounceDelay)
      debouncedState = reading;
    
    lastReading = reading;
  }

public:
  Button(int _pin, bool firstReading = HIGH)
    : lastDebounceTime(0),
      lastReading(firstReading),
      debouncedState(firstReading),
      lastPressed(false),
      pin(_pin) {}

  void init() {
    pinMode(pin, INPUT_PULLUP);
  }

  bool isPressed() {
    refresh();
    bool nowPressed = (debouncedState == LOW);
    bool edge = (!lastPressed && nowPressed);
    lastPressed = nowPressed;
    return edge;
  }

  bool isHeld() {
    refresh();
    return (debouncedState == LOW);
  }
};

#endif