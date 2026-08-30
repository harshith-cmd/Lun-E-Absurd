#include "drive.h"
#include <esp_random.h>

// The joystick's throttle/turn axes get run through one of these before
// touching the motors, picked once at boot. None of them is "correct".
typedef enum {
  SCR_INVERT_THROTTLE,   // forward becomes reverse
  SCR_INVERT_TURN,       // left becomes right
  SCR_INVERT_BOTH,       // forward-left becomes reverse-right
  SCR_SWAP,              // forward becomes a turn, turning becomes forward
  SCR_SWAP_INVERT_A,
  SCR_SWAP_INVERT_B,
  SCR_SWAP_INVERT_BOTH,
  SCR_COUNT
} ScrambleMode;

static ScrambleMode scrambleMode;

static void scramble(int throttle, int turn, int *outThrottle, int *outTurn) {
  switch (scrambleMode) {
    case SCR_INVERT_THROTTLE:  *outThrottle = -throttle; *outTurn =  turn;      break;
    case SCR_INVERT_TURN:      *outThrottle =  throttle; *outTurn = -turn;      break;
    case SCR_INVERT_BOTH:      *outThrottle = -throttle; *outTurn = -turn;      break;
    case SCR_SWAP:             *outThrottle =  turn;     *outTurn =  throttle; break;
    case SCR_SWAP_INVERT_A:    *outThrottle =  turn;     *outTurn = -throttle; break;
    case SCR_SWAP_INVERT_B:    *outThrottle = -turn;     *outTurn =  throttle; break;
    default:                   *outThrottle = -turn;     *outTurn = -throttle; break;
  }
}

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
  scrambleMode = (ScrambleMode)(esp_random() % SCR_COUNT);
  stopMotors();
}

void drive(int throttle, int turn) {
  curThrottle = throttle;
  curTurn = turn;
  pktCount++;
  lastPktTime = millis();

  int scThrottle, scTurn;
  scramble(throttle, turn, &scThrottle, &scTurn);

  int left = constrain(scThrottle + scTurn, -100, 100);
  int right = constrain(scThrottle - scTurn, -100, 100);
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
