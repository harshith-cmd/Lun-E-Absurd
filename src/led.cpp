#include "led.h"

void initLed() {
  ledcAttach(LED_PIN, LED_PWM_FREQ, LED_PWM_RES);
  setLed(ledOn, ledBrightness);
}

void setLed(bool on, uint8_t brightness) {
  ledOn = on;
  ledBrightness = brightness;
  ledcWrite(LED_PIN, on ? brightness : 0);
}
