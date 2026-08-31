# Zero Utility Vehicle

A remote-controlled rover with a WiFi joystick. No camera feed, no autonomy, no dialogue, no personality — just a joystick that drives the motors.

Built on the Seeed Studio XIAO ESP32-S3 Sense. The camera on the board is not used.

## What it actually does

1. Boots and starts a WiFi access point + web server.
2. Serves a single page with a virtual joystick at `/`.
3. The joystick streams `{throttle, turn}` to the rover over a WebSocket, 20 times a second.
4. `drive()` maps throttle/turn to left/right motor speed and drives the DRV8833.
5. It also listens for ESP-NOW broadcast `ControlPacket`s, so a separate physical remote can drive it on the same channel.
6. Stops the motors if no command arrives for 500 ms.

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | Seeed Studio XIAO ESP32-S3 Sense |
| Drive | DRV8833 dual H-bridge, 2 DC motors |
| Motor A | GPIO 43 / 44 |
| Motor B | GPIO 7 / 8 |
| Headlight LED | GPIO 6 (always on) |
| Power button | GPIO 2 (deep sleep) |

## Using it

1. Flash it, power it on.
2. Connect to the `Zero Utility Vehicle` WiFi network (password: `explorer123`).
3. Open `http://192.168.4.1/`.
4. Drag the joystick to drive.

## Control protocol

`ControlPacket` (`src/config.h`), same struct used by the web joystick internally and by ESP-NOW:

```c
typedef struct {
  int8_t throttle;  // -100..100
  int8_t turn;      // -100..100
  uint8_t flags;    // unused
} ControlPacket;
```

## Safety

Motors stop automatically if no control packet arrives within `DRIVE_TIMEOUT_MS` (500 ms, `config.h`).

## Project structure

```
src/
├── main.cpp        Entry point, shared state, safety timeout loop
├── config.h        Pin definitions, motor tuning, ControlPacket, network defaults
├── drive.cpp/h     Differential drive: drive(throttle, turn) → motor PWM
├── led.cpp/h       Headlight PWM (on at full brightness by default)
├── network.cpp/h   WiFi AP, web server, joystick WebSocket, ESP-NOW pairing
├── page_index.h    Joystick UI (HTML/CSS/JS in PROGMEM)
└── sleep.cpp/h     Deep sleep on button press
```

## Building

Requires [PlatformIO](https://platformio.org/).

```bash
pio run                    # Build
pio run --target upload    # Flash
pio device monitor         # Serial output
```
