/**********************************************************************
  Filename    : uno_rover_hold_receiver.ino
  Project     : Marc_Experiments/04_hold_control_watchdog
  Description : UNO rover motor control using hold-to-drive commands
                from ESP32, with a failsafe watchdog.

  Behavior:
    F = start/continue forward
    B = start/continue backward
    L = start/continue left/spin-left
    R = start/continue right/spin-right
    S = stop immediately

  Failsafe:
    If the UNO does not receive another movement command before the
    watchdog timeout expires, it auto-stops.

  Working wiring for camera web control:
    ESP GPIO15 -> UNO RX
    ESP GND    -> UNO GND
    Baud       -> 9600
**********************************************************************/

#define MOTOR_DIRECTION     0 // If direction is reversed, change 0 to 1
#define PIN_DIRECTION_LEFT  4
#define PIN_DIRECTION_RIGHT 3
#define PIN_MOTOR_PWM_LEFT  6
#define PIN_MOTOR_PWM_RIGHT 5
#define MOTOR_PWM_DEAD      10

#define DRIVE_SPEED         100
#define TURN_SPEED          100
#define WATCHDOG_TIMEOUT_MS 500

char activeCommand = 'S';
unsigned long lastCommandMs = 0;
bool moving = false;

void setup() {
  pinsSetup();
  Serial.begin(9600);
  resetCarAction();
  Serial.println("UNO rover receiver ready - HOLD CONTROL + WATCHDOG MODE");
}

void loop() {
  while (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }

  if (moving && millis() - lastCommandMs > WATCHDOG_TIMEOUT_MS) {
    resetCarAction();
    moving = false;
    activeCommand = 'S';
    Serial.println("WATCHDOG AUTO-STOP");
  }
}

void handleCommand(char cmd) {
  cmd = toupper(cmd);

  switch (cmd) {
    case 'F':
      startOrRefreshMove('F', DRIVE_SPEED, DRIVE_SPEED);
      break;

    case 'B':
      startOrRefreshMove('B', -DRIVE_SPEED, -DRIVE_SPEED);
      break;

    case 'L':
      startOrRefreshMove('L', -TURN_SPEED, TURN_SPEED);
      break;

    case 'R':
      startOrRefreshMove('R', TURN_SPEED, -TURN_SPEED);
      break;

    case 'S':
      Serial.println("ACK: S = stop now");
      resetCarAction();
      moving = false;
      activeCommand = 'S';
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

void startOrRefreshMove(char cmd, int leftSpeed, int rightSpeed) {
  lastCommandMs = millis();

  // Only print/start motor again when the command changes or we were stopped.
  // Repeated hold-refresh packets keep the watchdog alive without spamming serial.
  if (!moving || activeCommand != cmd) {
    activeCommand = cmd;
    moving = true;
    motorRun(leftSpeed, rightSpeed);

    Serial.print("ACK: ");
    Serial.print(cmd);
    Serial.println(" = move active");
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
