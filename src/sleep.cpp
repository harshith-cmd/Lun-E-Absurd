#include "sleep.h"
#include "drive.h"
#include "led.h"
#include <esp_sleep.h>
#include <WiFi.h>

static int buttonState = LOW;
static int lastRawButtonState = LOW;
static unsigned long lastDebounceTime = 0;

static void goToSleep() {
  Serial.println("[PWR] Button pressed: shutting down and entering deep sleep...");
  stopMotors();
  setLed(false, ledBrightness);
  WiFi.mode(WIFI_OFF);
  Serial.flush();

  // Wait for the button to be released, otherwise the still-HIGH pin
  // would immediately satisfy the wakeup condition below.
  while (digitalRead(BUTTON_PIN) == HIGH) {
    delay(10);
  }
  delay(BUTTON_DEBOUNCE_MS);

  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 1); // wake when HIGH
  esp_deep_sleep_start(); // does not return; resets on wake
}

void initSleep() {
  pinMode(BUTTON_PIN, INPUT); // external pulldown already sets the idle state
}

void sleepLoop() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastRawButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) { // button pressed
        goToSleep();
      }
    }
  }

  lastRawButtonState = reading;
}
