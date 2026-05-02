/**********************************************************************
  Filename    : uno_rover_receiver.ino
  Project     : Marc_Experiments/02_rover_serial_control
  Description : SAFE acknowledgement test for serial commands from ESP32.

  This version DOES NOT move the rover motors.
  It only receives commands and prints acknowledgement messages.

  Working wiring:
    ESP GPIO5 -> UNO RX
    ESP GND   -> UNO GND

  Commands:
    F = forward command received
    B = backward command received
    L = left command received
    R = right command received
    S = stop command received
**********************************************************************/

void setup() {
  Serial.begin(9600);
  Serial.println("UNO rover receiver ready - ACK ONLY MODE");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    acknowledgeCommand(cmd);
  }
}

void acknowledgeCommand(char cmd) {
  switch (cmd) {
    case 'F':
    case 'f':
      Serial.println("ACK: F = forward command received");
      break;

    case 'B':
    case 'b':
      Serial.println("ACK: B = backward command received");
      break;

    case 'L':
    case 'l':
      Serial.println("ACK: L = left command received");
      break;

    case 'R':
    case 'r':
      Serial.println("ACK: R = right command received");
      break;

    case 'S':
    case 's':
      Serial.println("ACK: S = stop command received");
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
