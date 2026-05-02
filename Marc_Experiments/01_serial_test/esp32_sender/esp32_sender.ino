#include <HardwareSerial.h>

// ESP32-WROVER -> UNO one-way serial test
// Working wiring:
//   ESP GPIO5 -> UNO RX
//   ESP GND   -> UNO GND
// Baud: 9600

HardwareSerial UnoSerial(1);

void setup() {
  Serial.begin(115200); // ESP USB debug serial to PC

  // Freenove ESP32-WROVER-DEV:
  // RX is unused for this one-way test (-1)
  // TX is GPIO5
  UnoSerial.begin(9600, SERIAL_8N1, -1, 5);

  Serial.println("ESP sender started");
}

void loop() {
  UnoSerial.println("F");
  Serial.println("Sent: F");
  delay(1000);
}
