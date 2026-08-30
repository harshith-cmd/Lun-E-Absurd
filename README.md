# LUN-E

A remote-controlled rover. There's a WiFi joystick and everything. It just doesn't listen to it — on every boot it picks a random (but consistent for that session) way of getting the controls wrong, so forward might come out as reverse, left might come out as right, or "throttle" and "turn" might swap outright and pushing forward makes it spin in place instead.

No camera feed, no autonomy, no dialogue, no personality — just a joystick, connected to a rover that interprets it badly.

Built on the Seeed Studio XIAO ESP32-S3 Sense. The camera on the board is not used.

## What it actually does

1. Boots and starts a WiFi access point + web server.
2. Serves a single page with a virtual joystick at `/`.
3. The joystick streams `{throttle, turn}` to the rover over a WebSocket, 20 times a second.
4. At boot, the rover randomly picked one of 7 ways to scramble that input (invert an axis, invert both, swap them, swap-and-invert) before touching the motors. Which one, you find out by driving into a wall.
5. It also still listens for ESP-NOW broadcast `ControlPacket`s, in case you'd rather build a second, different, equally-scrambled remote.
6. Stops the motors if no command arrives for 500 ms, so at least the wrongness has a timeout.

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | Seeed Studio XIAO ESP32-S3 Sense |
| Drive | DRV8833 dual H-bridge, 2 DC motors |
| Motor A | GPIO 43 / 44 |
| Motor B | GPIO 7 / 8 |
| Headlight LED | GPIO 6 |
| Power button | GPIO 2 (deep sleep) |

## Using it

1. Flash it, power it on.
2. Connect to the `LUN-E` WiFi network (password: `explorer123`).
3. Open `http://192.168.4.1/`.
4. Drag the joystick. Watch it do something else.

## Control protocol

`ControlPacket` (`src/config.h`), same struct used by the web joystick internally and by ESP-NOW:

```c
typedef struct {
  int8_t throttle;  // -100..100
  int8_t turn;      // -100..100
  uint8_t flags;    // unused
} ControlPacket;
```

The scrambling happens firmware-side in `drive()`, so it applies no matter which control path sends the command — there is no "correct" way to drive it from any interface.

## Safety

Motors stop automatically if no control packet arrives within `DRIVE_TIMEOUT_MS` (500 ms, `config.h`). This is the one part of the rover that behaves exactly as expected.

## Project structure

```
src/
├── main.cpp        Entry point, shared state, safety timeout loop
├── config.h        Pin definitions, motor tuning, ControlPacket, network defaults
├── drive.cpp/h     Differential drive + the axis-scrambler, → motor PWM
├── led.cpp/h       Headlight PWM (on/off + brightness, no control path exposed)
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
