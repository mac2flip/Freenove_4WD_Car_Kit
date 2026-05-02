/**********************************************************************
  Filename    : uno_rover_receiver.ino
  Project     : Marc_Experiments/02_rover_serial_control
  Description : Minimal UNO rover motor control using serial commands from ESP32.

  Working wiring:
    ESP GPIO5 -> UNO RX
    ESP GND   -> UNO GND

  Commands:
    F = forward
    B = backward
    L = left / spin left
    R = right / spin right
    S = stop
**********************************************************************/

#define MOTOR_DIRECTION     0 // If direction is reversed, change 0 to 1
#define PIN_DIRECTION_LEFT  4
#define PIN_DIRECTION_RIGHT 3
#define PIN_MOTOR_PWM_LEFT  6
#define PIN_MOTOR_PWM_RIGHT 5
#define MOTOR_PWM_DEAD      10

#define DRIVE_SPEED         120
#define TURN_SPEED          120

void setup() {
  pinsSetup();
  Serial.begin(9600);
  resetCarAction();
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }
}

void handleCommand(char cmd) {
  switch (cmd) {
    case 'F':
    case 'f':
      motorRun(DRIVE_SPEED, DRIVE_SPEED);
      break;

    case 'B':
    case 'b':
      motorRun(-DRIVE_SPEED, -DRIVE_SPEED);
      break;

    case 'L':
    case 'l':
      motorRun(-TURN_SPEED, TURN_SPEED);
      break;

    case 'R':
    case 'r':
      motorRun(TURN_SPEED, -TURN_SPEED);
      break;

    case 'S':
    case 's':
    case '\n':
    case '\r':
      resetCarAction();
      break;

    default:
      // Ignore unknown characters.
      break;
  }
}

void pinsSetup() {
  pinMode(PIN_DIRECTION_LEFT, OUTPUT);
  pinMode(PIN_MOTOR_PWM_LEFT, OUTPUT);
  pinMode(PIN_DIRECTION_RIGHT, OUTPUT);
  pinMode(PIN_MOTOR_PWM_RIGHT, OUTPUT);
}

void motorRun(int speedl, int speedr) {
  int dirL = 0, dirR = 0;

  if (speedl > 0) {
    dirL = 0 ^ MOTOR_DIRECTION;
  } else {
    dirL = 1 ^ MOTOR_DIRECTION;
    speedl = -speedl;
  }

  if (speedr > 0) {
    dirR = 1 ^ MOTOR_DIRECTION;
  } else {
    dirR = 0 ^ MOTOR_DIRECTION;
    speedr = -speedr;
  }

  speedl = constrain(speedl, 0, 255);
  speedr = constrain(speedr, 0, 255);

  if (abs(speedl) < MOTOR_PWM_DEAD && abs(speedr) < MOTOR_PWM_DEAD) {
    speedl = 0;
    speedr = 0;
  }

  digitalWrite(PIN_DIRECTION_LEFT, dirL);
  digitalWrite(PIN_DIRECTION_RIGHT, dirR);
  analogWrite(PIN_MOTOR_PWM_LEFT, speedl);
  analogWrite(PIN_MOTOR_PWM_RIGHT, speedr);
}

void resetCarAction() {
  motorRun(0, 0);
}
