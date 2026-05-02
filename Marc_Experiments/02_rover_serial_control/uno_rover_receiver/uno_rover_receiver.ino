/**********************************************************************
  Filename    : uno_rover_receiver.ino
  Project     : Marc_Experiments/02_rover_serial_control
  Description : SAFE pulse movement test for serial commands from ESP32.

  This version moves the rover only in short pulses, then auto-stops.
  Use with wheels lifted first.

  Working wiring:
    ESP GPIO5 -> UNO RX
    ESP GND   -> UNO GND

  Commands:
    F = forward pulse
    B = backward pulse
    L = left/spin-left pulse
    R = right/spin-right pulse
    S = stop immediately
**********************************************************************/

#define MOTOR_DIRECTION     0 // If direction is reversed, change 0 to 1
#define PIN_DIRECTION_LEFT  4
#define PIN_DIRECTION_RIGHT 3
#define PIN_MOTOR_PWM_LEFT  6
#define PIN_MOTOR_PWM_RIGHT 5
#define MOTOR_PWM_DEAD      10

#define DRIVE_SPEED         100
#define TURN_SPEED          100
#define DRIVE_PULSE_MS      200
#define TURN_PULSE_MS       150

unsigned long stopAtMs = 0;
bool pulseActive = false;

void setup() {
  pinsSetup();
  Serial.begin(9600);
  resetCarAction();
  Serial.println("UNO rover receiver ready - SAFE PULSE MODE");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }

  if (pulseActive && millis() >= stopAtMs) {
    resetCarAction();
    pulseActive = false;
    Serial.println("AUTO-STOP");
  }
}

void handleCommand(char cmd) {
  switch (cmd) {
    case 'F':
    case 'f':
      Serial.println("ACK: F = forward pulse");
      startPulse(DRIVE_SPEED, DRIVE_SPEED, DRIVE_PULSE_MS);
      break;

    case 'B':
    case 'b':
      Serial.println("ACK: B = backward pulse");
      startPulse(-DRIVE_SPEED, -DRIVE_SPEED, DRIVE_PULSE_MS);
      break;

    case 'L':
    case 'l':
      Serial.println("ACK: L = left pulse");
      startPulse(-TURN_SPEED, TURN_SPEED, TURN_PULSE_MS);
      break;

    case 'R':
    case 'r':
      Serial.println("ACK: R = right pulse");
      startPulse(TURN_SPEED, -TURN_SPEED, TURN_PULSE_MS);
      break;

    case 'S':
    case 's':
      Serial.println("ACK: S = stop now");
      resetCarAction();
      pulseActive = false;
      break;

    case '\n':
    case '\r':
      // Ignore line endings.
      break;

    default:
      Serial.print("UNKNOWN: ");
      Serial.println(cmd);
      break;
  }
}

void startPulse(int leftSpeed, int rightSpeed, unsigned long durationMs) {
  motorRun(leftSpeed, rightSpeed);
  stopAtMs = millis() + durationMs;
  pulseActive = true;
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
