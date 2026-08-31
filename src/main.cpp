#include <Arduino.h>
#include "config.h"
#include "drive.h"
#include "led.h"
#include "network.h"
#include "sleep.h"

// ===== SHARED STATE (defined here, declared extern in config.h) =====
volatile int curThrottle = 0, curTurn = 0;
volatile int curLeftSpd = 0, curRightSpd = 0;
volatile unsigned long pktCount = 0, lastPktTime = 0;
uint8_t activeChannel = DEFAULT_CHANNEL;
volatile bool ledOn = true;
volatile uint8_t ledBrightness = LED_DEFAULT_BRIGHTNESS;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n===== REMOTELY USELESS =====");

  initDrive();
  Serial.println("[DRV] OK");

  initLed();
  Serial.println("[LED] OK");

  setupNetwork();
  startWebServer();

  initSleep();
  Serial.println("[PWR] OK");

  Serial.println("[RDY] Remotely Useless online");
}

void loop() {
  networkLoop();
  sleepLoop();

  // Safety timeout: stop motors if no command for 500ms
  if (lastPktTime > 0 && millis() - lastPktTime > DRIVE_TIMEOUT_MS) {
    if (curThrottle != 0 || curTurn != 0) {
      stopMotors();
    }
  }
}
