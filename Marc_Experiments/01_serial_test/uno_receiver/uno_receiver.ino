// UNO receiver test for ESP32 -> UNO serial
// Working wiring:
//   ESP GPIO5 -> UNO RX
//   ESP GND   -> UNO GND
// Baud: 9600

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    Serial.print("Received: ");
    Serial.println(c);
  }
}
