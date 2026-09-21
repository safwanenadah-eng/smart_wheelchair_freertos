# 🦼 Smart Bluetooth-Controlled Wheelchair (FreeRTOS)

Real-time embedded system for wireless wheelchair control, developed on an **ATmega328P** microcontroller using a **FreeRTOS** multitasking architecture. Features include ultrasonic collision avoidance, progressive speed ramp control (acceleration/deceleration), and Bluetooth connectivity for smartphone control.

Fully simulated in **Proteus 8 Professional**.

---

## 📌 Key Features

- **Wireless Remote Control**: Controlled via Bluetooth (HC-05 module) using standard commands: `F` (forward), `B` (backward), `L` (left), `R` (right), `S` (stop).
- **FreeRTOS Multitasking Architecture**: 3 concurrent tasks running with distinct priorities (safety, communication, motor control).
- **Collision Avoidance System**: Obstacle detection via HC-SR04 ultrasonic sensor featuring emergency stop with hysteresis (stop < 30 cm, resume > 40 cm) and a median filter for noise suppression.
- **Progressive Motor Control**: Smooth acceleration and deceleration ramps to eliminate abrupt velocity jumps, coupled with active braking (H-Bridge short-circuit) for rapid emergency stops.
- **Direction Inversion Protection**: Mandatory automatic stop before changing rotation direction (forward ↔ reverse).
- **Real-Time Telemetry Feedback**: Live transmission of real speed (km/h) and obstacle distance via Bluetooth.

---

## 🏗️ Software Architecture (FreeRTOS)

The system relies on 3 tasks synchronized through a shared message queue:
