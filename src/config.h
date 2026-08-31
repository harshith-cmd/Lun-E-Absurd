#pragma once
#include <Arduino.h>

// ===== DRIVE MOTORS (DRV8833 dual H-bridge) =====
#define MOTOR_A_IN1      43   // M1
#define MOTOR_A_IN2      44   // M1'
#define MOTOR_B_IN1      7    // M2
#define MOTOR_B_IN2      8    // M2'
#define MOTOR_PWM_FREQ   20000
#define MOTOR_PWM_RES    8
#define MOTOR_MAX_DUTY   ((1 << MOTOR_PWM_RES) - 1)

// ===== HEADLIGHT LED =====
#define LED_PIN          6
#define LED_PWM_FREQ     5000
#define LED_PWM_RES      8
#define LED_DEFAULT_BRIGHTNESS 255

// ===== NETWORK =====
#define DEFAULT_CHANNEL  6
#define AP_SSID          "Remotely Useless"
#define AP_PASS          "explorer123"

// ===== SAFETY =====
#define DRIVE_TIMEOUT_MS 500

// ===== POWER BUTTON (deep sleep) =====
// External 10k pulldown to GND, connects to 3V3 when pressed: active-HIGH.
#define BUTTON_PIN         2
#define BUTTON_DEBOUNCE_MS 50

// ===== SHARED STATE =====
typedef struct {
  int8_t throttle;
  int8_t turn;
  uint8_t flags;
} ControlPacket;

extern volatile int curThrottle, curTurn;
extern volatile int curLeftSpd, curRightSpd;
extern volatile unsigned long pktCount, lastPktTime;
extern uint8_t activeChannel;
extern volatile bool ledOn;
extern volatile uint8_t ledBrightness;
