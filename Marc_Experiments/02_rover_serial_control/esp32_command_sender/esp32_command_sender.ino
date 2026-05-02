#include <HardwareSerial.h>

/**********************************************************************
  Filename    : esp32_command_sender.ino
  Project     : Marc_Experiments/02_rover_serial_control
  Description : ESP32 sends simple movement commands to the UNO rover.

  Working wiring:
    ESP GPIO5 -> UNO RX
    ESP GND   -> UNO GND

  Commands sent to UNO:
    F = forward
    B = backward
    L = left / spin left
    R = right / spin right
    S = stop
**********************************************************************/

HardwareSerial UnoSerial(1);

void setup() {
  Serial.begin(115200); // ESP USB debug serial to PC

  // Freenove ESP32-WROVER-DEV:
  // RX is unused for this one-way test (-1)
  // TX is GPIO5
  UnoSerial.begin(9600, SERIAL_8N1, -1, 5);

  Serial.println("ESP32 command sender started");
  Serial.println("Command cycle: F, S, B, S, L, S, R, S");
}

void loop() {
  sendCommand('F');
  delay(1000);
  sendCommand('S');
  delay(500);

  sendCommand('B');
  delay(1000);
  sendCommand('S');
  delay(500);

  sendCommand('L');
  delay(700);
  sendCommand('S');
  delay(500);

  sendCommand('R');
  delay(700);
  sendCommand('S');
  delay(1500);
}

void sendCommand(char cmd) {
  UnoSerial.write(cmd);
  Serial.print("Sent: ");
  Serial.println(cmd);
}
