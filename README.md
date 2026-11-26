# IoT-Enabled Intelligent Traffic Signal System

> **ESP32 | Embedded C/C++ | IR Sensors | RFID | State Machine**

An ESP32-based intelligent traffic management system that dynamically adjusts signal timings based on real-time vehicle density and provides emergency vehicle priority using RFID detection.

---

## 🎯 Features

- **Dynamic Signal Timing** — Adjusts green signal duration (5–30s) based on real-time vehicle density measured by IR sensors
- **Emergency Vehicle Priority** — RFID-based detection instantly overrides normal traffic cycles to provide priority green signals for emergency vehicles
- **State Machine Architecture** — Clean, maintainable finite state machine controlling all traffic phases (Green → Yellow → All-Red → next lane)
- **4-Way Intersection Control** — Manages North, East, South, and West lanes independently
- **Density-Aware Scheduling** — Prioritizes high-traffic lanes in the signal rotation cycle
- **Safety Interlocks** — All-red clearance phases between signal transitions, emergency cooldown timers

---

## 📁 Project Structure

```
IoT-Intelligent-Traffic-Signal/
├── src/
│   ├── main.cpp                    # Application entry point
│   ├── config.h                    # Pin definitions & constants
│   ├── ir_sensor.h                 # IR sensor array header
│   ├── ir_sensor.cpp               # Vehicle density measurement
│   ├── rfid_handler.h              # RFID handler header
│   ├── rfid_handler.cpp            # Emergency vehicle detection
│   ├── signal_controller.h         # LED signal controller header
│   ├── signal_controller.cpp       # Traffic light LED control
│   ├── traffic_state_machine.h     # Traffic FSM header
│   └── traffic_state_machine.cpp   # Core state machine logic
├── docs/
│   ├── CIRCUIT_WIRING.md           # Pin mapping & wiring guide
│   └── ARCHITECTURE.md             # System architecture & diagrams
├── platformio.ini                  # PlatformIO build configuration
├── .gitignore
├── LICENSE
└── README.md
```

---

## 🔧 Hardware Requirements

| Component | Qty | Purpose |
|-----------|-----|---------|
| ESP32 DevKit V1 | 1 | Main microcontroller |
| MFRC522 RFID Module | 1 | Emergency vehicle tag reader |
| IR Obstacle Sensor (FC-51) | 4 | Vehicle density sensing (1 per lane) |
| Red LED 5mm | 4 | Red traffic signals |
| Yellow LED 5mm | 4 | Yellow traffic signals |
| Green LED 5mm | 4 | Green traffic signals |
| 220Ω Resistors | 12 | LED current limiting |
| RFID Tags/Cards | 4+ | Emergency vehicle identification |
| Breadboard & Jumper Wires | — | Prototyping |

---

## ⚡ Pin Configuration

| Function | ESP32 GPIO |
|----------|-----------|
| **Lane 0 (North)** — R / Y / G | 2 / 4 / 5 |
| **Lane 1 (East)** — R / Y / G | 13 / 14 / 15 |
| **Lane 2 (South)** — R / Y / G | 16 / 17 / 18 |
| **Lane 3 (West)** — R / Y / G | 25 / 26 / 27 |
| **IR Sensors** — N / E / S / W | 34 / 35 / 32 / 33 |
| **RFID SDA / RST** | 21 / 22 |
| **RFID SPI** (SCK / MOSI / MISO) | 18 / 23 / 19 |

> See [`docs/CIRCUIT_WIRING.md`](docs/CIRCUIT_WIRING.md) for detailed wiring instructions.

---

## 🚀 Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- ESP32 board support package
- USB cable for ESP32

### Build & Upload (PlatformIO)

```bash
# Clone the repository
git clone https://github.com/<your-username>/IoT-Intelligent-Traffic-Signal.git
cd IoT-Intelligent-Traffic-Signal

# Build
pio run

# Upload to ESP32
pio run --target upload

# Open serial monitor
pio device monitor
```

### Build & Upload (Arduino IDE)

1. Install ESP32 board support via Board Manager
2. Install the **MFRC522** library by Miguel Balboa via Library Manager
3. Open `src/main.cpp` as a `.ino` file (rename if needed)
4. Place all `.h` and `.cpp` files in the same sketch folder
5. Select **ESP32 Dev Module** as the board
6. Upload and open Serial Monitor at 115200 baud

---

## 🔄 How It Works

### State Machine Flow

```
IDLE → GREEN_PHASE → YELLOW_PHASE → ALL_RED_PHASE → (next lane) GREEN_PHASE → ...
                          ↓ (RFID detected at any point)
                     EMERGENCY → EMERGENCY_CLEAR → (resume normal cycle)
```

### Dynamic Density Algorithm

1. IR sensors sample vehicle presence every 200ms
2. A sliding window (20 samples) computes density percentage per lane
3. Green time is linearly interpolated:
   - 0% density → 5s (minimum green)
   - 100% density → 30s (maximum green)
4. If a non-active lane exceeds 70% density, it gets scheduling priority

### Emergency Override

1. RFID reader polls every 100ms for new tags
2. When a registered emergency tag is scanned:
   - All lanes immediately go **RED**
   - The priority lane (mapped to the tag) gets **GREEN** for 15s
   - After 15s: Yellow transition → All-red → Resume normal cycle
3. A 5s cooldown prevents repeated triggers

---

## 🔧 Configuration

All timing constants and thresholds are in [`src/config.h`](src/config.h):

| Parameter | Default | Description |
|-----------|---------|-------------|
| `BASE_GREEN_TIME_MS` | 10,000 | Default green time (ms) |
| `MIN_GREEN_TIME_MS` | 5,000 | Minimum green time |
| `MAX_GREEN_TIME_MS` | 30,000 | Maximum green time |
| `YELLOW_TIME_MS` | 3,000 | Yellow transition duration |
| `ALL_RED_TIME_MS` | 2,000 | All-red clearance gap |
| `EMERGENCY_GREEN_TIME_MS` | 15,000 | Emergency override duration |
| `DENSITY_HIGH_THRESHOLD` | 70% | Threshold for "heavy traffic" |

### Registering Emergency Vehicle Tags

Edit `rfid_handler.cpp` → `begin()` to add your RFID tag UIDs:

```cpp
registerEmergencyTag("A3 B2 C1 D0", 0);  // UID → Lane 0 (North)
registerEmergencyTag("11 22 33 44", 1);  // UID → Lane 1 (East)
```

> To find your tag's UID, upload the code, scan the tag, and check the Serial Monitor output.

---

## 📊 Serial Monitor Output

```
╔══════════════════════════════════════════════╗
║       Traffic Signal System — Status         ║
╠══════════════════════════════════════════════╣
║  State:        GREEN_PHASE                   ║
║  Active Lane:  0 (North)                     ║
║  Green Time:   18500 ms                      ║
║  Phase Elapsed: 7230 ms                      ║
║  Total Cycles: 42                            ║
╚══════════════════════════════════════════════╝
── IR Sensor Density ──
  North : 65% [MEDIUM] | Vehicle: YES
  East  : 30% [MEDIUM] | Vehicle: NO
  South : 85% [HIGH  ] | Vehicle: YES
  West  : 10% [LOW   ] | Vehicle: NO
── Signal States ──
  North : GREEN
  East  : RED
  South : RED
  West  : RED
```

---

## 📐 Architecture

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for detailed system architecture, state diagrams, and module relationships.

---

## 🛠 Tech Stack

- **Microcontroller:** ESP32-WROOM-32 (Dual-core 240MHz, Wi-Fi, Bluetooth)
- **Language:** Embedded C/C++ (Arduino Framework)
- **Build System:** PlatformIO
- **RFID Library:** MFRC522 by Miguel Balboa
- **Sensors:** FC-51 IR Obstacle Sensors
- **Design Pattern:** Finite State Machine (FSM)

---

## 📄 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

---

## 👥 Authors

- IoT Project Team — Sep 2025 – Jan 2026

---

## 🙏 Acknowledgments

- [MFRC522 Arduino Library](https://github.com/miguelbalboa/rfid)
- [PlatformIO](https://platformio.org/)
- [Espressif ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
