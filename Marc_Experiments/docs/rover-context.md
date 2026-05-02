# Rover Context

## Hardware

- Freenove 4WD rover kit
- UNO controls rover motors
- Freenove ESP32-WROVER-DEV is mounted on rover
- ESP32 board does not expose GPIO17/GPIO16 in the visible header layout
- GPIO5 was confirmed working as UART TX

## Working Serial Link

- ESP GPIO5 -> UNO RX
- ESP GND -> UNO GND
- Baud: 9600
- ESP sends `F` once per second
- UNO receives `F` successfully
- ESP command cycle `F -> S -> B -> S -> L -> S -> R -> S` was confirmed by UNO ACK output

## Important Notes

- Do not use ESP pins labeled `TX`/`RX` for rover communication; those are shared with USB serial.
- Remove Bluetooth/RF module from UNO RX/TX while testing ESP serial.
- Upload UNO sketch before connecting ESP TX to UNO RX.
- Disconnect ESP GPIO5 from UNO RX before uploading sketches to the UNO, then reconnect it after upload.
- Current test is one-way only: ESP -> UNO.

## Confirmed ACK Test

UNO successfully acknowledged the ESP command cycle with messages like:

```text
ACK: L = left command received
ACK: S = stop command received
ACK: R = right command received
ACK: S = stop command received
ACK: F = forward command received
ACK: S = stop command received
ACK: B = backward command received
ACK: S = stop command received
```

## Goal

Use ESP WiFi controls to send single-character movement commands to UNO:

- `F` = forward
- `B` = backward
- `L` = left
- `R` = right
- `S` = stop

UNO should receive the character and call the existing Freenove motor movement functions.
