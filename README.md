# 🦼 Bluetooth-Controlled Smart Wheelchair

Real-time embedded system for wireless control of a wheelchair, built on an **ATmega328P** with a **FreeRTOS** multitasking architecture. The project includes ultrasonic anti-collision safety, progressive speed control (acceleration/deceleration ramp), and Bluetooth communication for smartphone control.

Fully simulated in **Proteus 8 Professional**.

---

## 📌 Features

- **Wireless control** via Bluetooth (HC-05 module), commands: `F` (forward), `B` (backward), `L` (left), `R` (right), `S` (stop)
- **FreeRTOS multitasking architecture**: 3 concurrent tasks with distinct priorities (safety, communication, motor control)
- **Anti-collision safety**: obstacle detection via HC-SR04 ultrasonic sensor, automatic stop with hysteresis (stop < 30 cm, resume > 40 cm), median filtering against sensor noise
- **Progressive motor control**: acceleration/deceleration ramp (no abrupt speed jumps), active braking (H-bridge short-circuit) for fast stopping
- **Direction-reversal safety**: automatic forced stop before any change of rotation direction (forward ↔ backward)
- **State feedback**: real speed (km/h) and obstacle distance transmitted in real time via Bluetooth

---

## 🏗️ Software Architecture (FreeRTOS)

The system relies on 3 tasks and a shared message queue:

```
               +-----------------------+
               | Smartphone (Bluetooth)|
               +-----------+-----------+
                           | (UART RX)
                           v
               +-----------------------+
               |  vTaskBluetoothRx     |  Priority 3
               +-----------+-----------+
                           |
                     [ xCmdQueue ]
                           |
                           v
+------------------+  +----+------------------+
| vTaskObstacle    |->| vTaskMotorControl     |  Priority 2
| Priority 4 (Safety)| +-----------------------+
+------------------+
```

| Task | Priority | Role |
|---|---|---|
| `vTaskObstacle` | 4 (highest) | Reads the ultrasonic sensor every 100 ms, emergency stop if obstacle < 30 cm |
| `vTaskBluetoothRx` | 3 | Receives UART commands, pushes them into the message queue |
| `vTaskMotorControl` | 2 | Applies the speed ramp and drives the H-bridge |

---

## 🔌 Wiring Diagram

| Signal | ATmega328P Pin | Component |
|---|---|---|
| RXD / TXD | PD0 / PD1 | Bluetooth HC-05 module (UART) |
| Trigger / Echo | PD2 / PD3 | HC-SR04 ultrasonic sensor |
| IN1 / IN4 | PD4 / PD7 | Motor direction (L298) |
| IN2 / IN3 | PB0 / PB3 | Motor direction (L298) |
| ENA / ENB | PD6 / PD5 (Timer0 PWM) | Motor speed (L298) |

> ⚠️ Motor PWM uses **Timer0** (not Timer1), since Timer1 is reserved for the FreeRTOS system tick.

### Hardware protection
- 8 flyback diodes (1N5819) on the 4 L298 outputs (protection against inductive voltage spikes)
- Decoupling capacitors: 100 nF (5V logic), 470 µF + 100 nF (motor power)
- Common ground across all blocks (logic, power, Bluetooth, sensor)

---

## 📁 Project Structure

```
smart_wheelchair_freertos/
├── FreeRTOS_Source/          # FreeRTOS kernel (GCC/ATmega328P port)
├── src/
│   ├── Makefile               # Multi-file compilation (avr-gcc)
│   ├── main.c                 # RTOS tasks, queue creation, scheduler
│   ├── bluetooth.c / .h       # UART / Bluetooth driver
│   ├── motors.c / .h          # PWM / L298 H-bridge driver, speed ramp
│   ├── ultrasonic.c / .h      # HC-SR04 sensor driver, median filtering
│   └── FreeRTOSConfig.h       # FreeRTOS kernel configuration
├── simulation/
│   └── wheelchair_schematic.pdsprj   # Full Proteus project
├── assets/                    # Screenshots / demo captures
├── README.md
└── .gitignore
```

---

## ⚙️ Build & Programming

### Requirements
- [AVR-GCC toolchain](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio) (or the one bundled with the Arduino IDE)
- `make` (available via MinGW / WSL / Git Bash on Windows)
- [Proteus 8 Professional](https://www.labcenter.com/) for simulation

### Build
```bash
cd src
make clean
make
```
Generates `src/main.hex`, ready to be loaded onto the ATmega328P (via Proteus or avrdude).

### Real hardware programming (optional)
```bash
make upload
```
(configure `PORT` and `BAUD` in the `Makefile` according to your setup)

---

## 🧪 Proteus Simulation

Components used:
- `ARDUINO UNO` (ATmega328P, 16 MHz)
- `COMPIM` or `Virtual Terminal` — Bluetooth link simulation
- `L298` — H-bridge for motor driving
- `MOTOR-DC` (x2) — left/right wheel motors
- `ULTRASONIC V2.0 B` — distance sensor (Arduino Proteus library)

Open `simulation/wheelchair_schematic.pdsprj`, load `main.hex` onto the ATmega328P, run the simulation, and send commands via the virtual terminal or a smartphone connected through COMPIM.

---

## 🎯 Notable Technical Points

- **FreeRTOS/Timer synchronization**: identified and resolved a conflict between the FreeRTOS tick (Timer1) and the motor PWM, fixed by migrating the PWM to Timer0.
- **Hysteresis-based safety**: eliminated erratic stop/resume oscillations near the detection threshold using two distinct thresholds (stop/resume) and median filtering over 3 samples.
- **Command vs. physical measurement distinction**: the displayed speed (`V:x.xkm/h`) reflects the PWM command sent, as opposed to the motor's actual mechanical response (inertia) — a fundamental distinction in physical system control.

---

## 📅 Project completed in 2026 — as part of a real-time embedded systems portfolio.
