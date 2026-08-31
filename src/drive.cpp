#include "drive.h"

// DRV8833: one side PWM'd for direction, the other held low. Both low = coast.
static void writeMotor(int pinFwd, int pinRev, int speed) {
  speed = constrain(speed, -MOTOR_MAX_DUTY, MOTOR_MAX_DUTY);
  if (speed >= 0) {
    ledcWrite(pinFwd, speed);
    ledcWrite(pinRev, 0);
  } else {
    ledcWrite(pinFwd, 0);
    ledcWrite(pinRev, -speed);
  }
}

void initDrive() {
  ledcAttach(MOTOR_A_IN1, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  ledcAttach(MOTOR_A_IN2, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  ledcAttach(MOTOR_B_IN1, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  ledcAttach(MOTOR_B_IN2, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  stopMotors();
}

void drive(int throttle, int turn) {
  curThrottle = throttle;
  curTurn = turn;
  pktCount++;
  lastPktTime = millis();

  int left = constrain(throttle + turn, -100, 100);
  int right = constrain(throttle - turn, -100, 100);
  // Motors are mirrored on the chassis, so the right side's polarity is inverted.
  curLeftSpd = map(left, -100, 100, -MOTOR_MAX_DUTY, MOTOR_MAX_DUTY);
  curRightSpd = map(right, -100, 100, MOTOR_MAX_DUTY, -MOTOR_MAX_DUTY);
  writeMotor(MOTOR_A_IN1, MOTOR_A_IN2, curLeftSpd);
  writeMotor(MOTOR_B_IN1, MOTOR_B_IN2, curRightSpd);
}

void stopMotors() {
  curThrottle = 0;
  curTurn = 0;
  curLeftSpd = 0;
  curRightSpd = 0;
  writeMotor(MOTOR_A_IN1, MOTOR_A_IN2, 0);
  writeMotor(MOTOR_B_IN1, MOTOR_B_IN2, 0);
}
