# 🦼 Fauteuil Roulant Intelligent Contrôlé par Bluetooth

Système embarqué temps réel pour le pilotage sans fil d'un fauteuil roulant, développé sur **ATmega328P** avec une architecture multitâche **FreeRTOS**. Le projet inclut une sécurité anti-collision par capteur ultrasonique, une gestion progressive de la vitesse (rampe d'accélération/décélération), et une communication Bluetooth pour le contrôle via smartphone.

Simulation complète réalisée sous **Proteus 8 Professional**.

---

## 📌 Fonctionnalités

- **Pilotage sans fil** via Bluetooth (module HC-05), commandes : `F` (avancer), `B` (reculer), `L` (gauche), `R` (droite), `S` (stop)
- **Architecture multitâche FreeRTOS** : 3 tâches concurrentes avec priorités distinctes (sécurité, communication, contrôle moteur)
- **Sécurité anti-collision** : détection d'obstacle par capteur ultrasonique HC-SR04, arrêt automatique avec hystérésis (arrêt < 30 cm, reprise > 40 cm), filtrage médian anti-bruit
- **Contrôle moteur progressif** : rampe d'accélération/décélération (pas de saut brutal de vitesse), freinage actif (court-circuit du pont en H) pour un arrêt rapide
- **Sécurité anti-inversion** : arrêt forcé automatique avant tout changement de sens de rotation (avant ↔ arrière)
- **Retour d'état** : vitesse réelle (km/h) et distance d'obstacle transmises en temps réel via Bluetooth

---

## 🏗️ Architecture Logicielle (FreeRTOS)

Le système repose sur 3 tâches et une file de messages partagée :

```
               +-----------------------+
               | Smartphone (Bluetooth)|
               +-----------+-----------+
                           | (UART RX)
                           v
               +-----------------------+
               |  vTaskBluetoothRx     |  Prio 3
               +-----------+-----------+
                           |
                     [ xCmdQueue ]
                           |
                           v
+------------------+  +----+------------------+
| vTaskObstacle    |->| vTaskMotorControl     |  Prio 2
| Prio 4 (Sécurité)|  +-----------------------+
+------------------+
```

| Tâche | Priorité | Rôle |
|---|---|---|
| `vTaskObstacle` | 4 (max) | Lecture capteur ultrason toutes les 100 ms, arrêt d'urgence si obstacle < 30 cm |
| `vTaskBluetoothRx` | 3 | Réception des commandes UART, dépôt dans la file de messages |
| `vTaskMotorControl` | 2 | Application de la rampe de vitesse et pilotage du pont en H |

---

## 🔌 Schéma de Câblage

| Signal | Broche ATmega328P | Composant |
|---|---|---|
| RXD / TXD | PD0 / PD1 | Module Bluetooth HC-05 (UART) |
| Trigger / Echo | PD2 / PD3 | Capteur ultrasonique HC-SR04 |
| IN1 / IN4 | PD4 / PD7 | Direction moteurs (L298) |
| IN2 / IN3 | PB0 / PB3 | Direction moteurs (L298) |
| ENA / ENB | PD6 / PD5 (Timer0 PWM) | Vitesse moteurs (L298) |

> ⚠️ Le PWM moteur utilise **Timer0** (et non Timer1) car Timer1 est réservé au tick système de FreeRTOS.

### Protection matérielle
- 8 diodes de roue libre (1N5819) sur les 4 sorties du L298 (protection contre les surtensions inductives)
- Condensateurs de découplage : 100 nF (logique 5V), 470 µF + 100 nF (puissance moteur)
- Masse commune entre tous les blocs (logique, puissance, Bluetooth, capteur)

---

## 📁 Structure du Projet

```
smart_wheelchair_freertos/
├── FreeRTOS_Source/          # Noyau FreeRTOS (portable GCC/ATmega328P)
├── src/ (racine du repo)
│   ├── main.c                # Tâches RTOS, création de la queue, scheduler
│   ├── bluetooth.c / .h      # Pilote UART / Bluetooth
│   ├── motors.c / .h         # Pilote PWM / pont en H L298, rampe de vitesse
│   ├── ultrasonic.c / .h     # Pilote capteur HC-SR04, filtrage médian
│   └── FreeRTOSConfig.h      # Configuration du noyau FreeRTOS
├── simulation/
│   └── wheelchair_schematic.pdsprj   # Projet Proteus complet
├── assets/                   # Captures d'écran / démonstration
├── Makefile                  # Compilation multi-fichiers (avr-gcc)
└── README.md
```

---

## ⚙️ Compilation & Programmation

### Prérequis
- [AVR-GCC toolchain](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio) (ou celle fournie avec l'IDE Arduino)
- `make` (disponible via MinGW / WSL / Git Bash sous Windows)
- [Proteus 8 Professional](https://www.labcenter.com/) pour la simulation

### Compilation
```bash
make clean
make
```
Génère `main.hex`, prêt à être chargé sur l'ATmega328P (via Proteus ou avrdude).

### Programmation réelle (optionnel)
```bash
make upload
```
(configurer `PORT` et `BAUD` dans le `Makefile` selon votre installation)

---

## 🧪 Simulation Proteus

Composants utilisés :
- `ARDUINO UNO` (ATmega328P, 16 MHz)
- `COMPIM` ou `Virtual Terminal` — simulation de la liaison Bluetooth
- `L298` — pont en H pour le pilotage moteur
- `MOTOR-DC` (x2) — moteurs des roues gauche/droite
- `ULTRASONIC V2.0 B` — capteur de distance (bibliothèque Arduino Proteus)

Ouvrir `simulation/wheelchair_schematic.pdsprj`, charger `main.hex` sur l'ATmega328P, lancer la simulation, et envoyer les commandes via le terminal virtuel ou un smartphone connecté via COMPIM.

---

## 🎯 Points Techniques Notables

- **Synchronisation FreeRTOS/Timer** : identification et résolution d'un conflit entre le tick FreeRTOS (Timer1) et le PWM moteur, résolu en migrant le PWM vers Timer0.
- **Sécurité par hystérésis** : élimination des oscillations d'arrêt/redémarrage près du seuil de détection grâce à deux seuils distincts (arrêt/reprise) et un filtrage médian sur 3 échantillons.
- **Distinction commande / mesure physique** : la vitesse affichée (`V:x.xkm/h`) reflète la consigne PWM envoyée, à différencier de la réponse mécanique réelle du moteur (inertie), une distinction fondamentale en contrôle de systèmes physiques.

---

## 📅 Projet réalisé en 2026 — dans le cadre d'un portfolio de systèmes embarqués temps réel.