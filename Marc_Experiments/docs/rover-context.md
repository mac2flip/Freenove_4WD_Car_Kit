# Rover Context

## Hardware

- Freenove 4WD rover kit
- UNO controls rover motors
- Freenove ESP32-WROVER-DEV is mounted on rover
- ESP32 board does not expose GPIO17/GPIO16 in the visible header layout
- GPIO5 was confirmed working as UART TX for serial-only testing
- GPIO15 was confirmed working as UART TX while the camera is active

## Working Serial Link

- Serial-only test: ESP GPIO5 -> UNO RX
- Camera web-control test: ESP GPIO15 -> UNO RX
- ESP GND -> UNO GND
- Baud: 9600
- ESP sends `F` once per second
- UNO receives `F` successfully
- ESP command cycle `F -> S -> B -> S -> L -> S -> R -> S` was confirmed by UNO ACK output
- Safe pulse mode was confirmed: UNO receives movement commands, runs short pulses, and auto-stops
- Camera web-control mode was confirmed: browser buttons on the ESP32 web page send commands to the UNO successfully

## Important Notes

- Do not use ESP pins labeled `TX`/`RX` for rover communication; those are shared with USB serial.
- Remove Bluetooth/RF module from UNO RX/TX while testing ESP serial.
- Upload UNO sketch before connecting ESP TX to UNO RX.
- Disconnect the ESP UART TX wire from UNO RX before uploading sketches to the UNO, then reconnect it after upload.
- Current test is one-way only: ESP -> UNO.
- Rover motors do not move when powered only by USB-C/USB; battery power is required for actual motor movement.
- GPIO5 conflicts with the WROVER camera pin map, so do not use GPIO5 for UART when the camera is active.

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

## Confirmed Safe Pulse Test

UNO successfully acknowledged movement commands, started short pulses, and auto-stopped:

```text
ACK: L = left pulse
AUTO-STOP
ACK: S = stop now
ACK: R = right pulse
AUTO-STOP
ACK: S = stop now
ACK: F = forward pulse
AUTO-STOP
ACK: S = stop now
ACK: B = backward pulse
AUTO-STOP
```

## Confirmed Camera Web Control Test

The ESP32 camera sketch was extended with a rover control web page on port 82.

- Camera page: `http://<ESP32-IP>/`
- Rover control page: `http://<ESP32-IP>:82/`
- Command endpoint: `http://<ESP32-IP>:82/cmd?move=F`

Browser buttons successfully sent movement commands to the UNO.

## Goal

Use ESP WiFi controls to send single-character movement commands to UNO:

- `F` = forward
- `B` = backward
- `L` = left
- `R` = right
- `S` = stop

UNO should receive the character and call the existing Freenove motor movement functions.
