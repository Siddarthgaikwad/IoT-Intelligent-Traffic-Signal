# Circuit Wiring Guide

## Components Required

| Component | Quantity | Description |
|-----------|----------|-------------|
| ESP32 DevKit V1 | 1 | Main microcontroller |
| MFRC522 RFID Module | 1 | Emergency vehicle detection |
| IR Obstacle Sensor (FC-51) | 4 | Vehicle density detection (1 per lane) |
| Red LED (5mm) | 4 | Red signal for each lane |
| Yellow LED (5mm) | 4 | Yellow signal for each lane |
| Green LED (5mm) | 4 | Green signal for each lane |
| 220Ω Resistor | 12 | Current limiting for LEDs |
| RFID Cards/Tags | 4+ | Emergency vehicle identification |
| Breadboard | 2 | Prototyping |
| Jumper Wires | ~50 | Connections |
| 5V Power Supply | 1 | Power for ESP32 and peripherals |

---

## ESP32 Pin Mapping

### Traffic Signal LEDs

| Lane | Direction | Red Pin | Yellow Pin | Green Pin |
|------|-----------|---------|------------|-----------|
| 0 | North | GPIO 2 | GPIO 4 | GPIO 5 |
| 1 | East | GPIO 13 | GPIO 14 | GPIO 15 |
| 2 | South | GPIO 16 | GPIO 17 | GPIO 18 |
| 3 | West | GPIO 25 | GPIO 26 | GPIO 27 |

### IR Sensors

| Lane | Direction | Signal Pin |
|------|-----------|------------|
| 0 | North | GPIO 34 |
| 1 | East | GPIO 35 |
| 2 | South | GPIO 32 |
| 3 | West | GPIO 33 |

> **Note:** GPIO 34, 35 are input-only pins on ESP32 — perfect for sensor inputs.

### RFID (MFRC522)

| MFRC522 Pin | ESP32 Pin |
|-------------|-----------|
| SDA (SS) | GPIO 21 |
| SCK | GPIO 18* |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| RST | GPIO 22 |
| 3.3V | 3.3V |
| GND | GND |

> **⚠️ Note:** GPIO 18 is shared between Lane 2 Green LED and SPI SCK. If this causes conflict, reassign Lane 2 Green to another available GPIO (e.g., GPIO 12) and update `config.h`.

---

## Wiring Diagram (Text)

```
                        ┌─────────────────┐
                        │    ESP32 DevKit  │
                        │                  │
   Lane 0 (North)       │                  │       MFRC522 RFID
   ┌─────────┐          │                  │      ┌──────────┐
   │ RED LED ├──220Ω──► GPIO 2        GPIO 21◄───┤ SDA      │
   │ YEL LED ├──220Ω──► GPIO 4        GPIO 22◄───┤ RST      │
   │ GRN LED ├──220Ω──► GPIO 5        GPIO 23◄───┤ MOSI     │
   └─────────┘          │              GPIO 19◄───┤ MISO     │
                        │              GPIO 18◄───┤ SCK      │
   Lane 1 (East)        │              3.3V  ◄───┤ VCC      │
   ┌─────────┐          │              GND   ◄───┤ GND      │
   │ RED LED ├──220Ω──► GPIO 13       └──────────┘
   │ YEL LED ├──220Ω──► GPIO 14       │
   │ GRN LED ├──220Ω──► GPIO 15       │
   └─────────┘          │              │
                        │              │
   Lane 2 (South)       │         IR Sensors
   ┌─────────┐          │        ┌────────────┐
   │ RED LED ├──220Ω──► GPIO 16  │ IR Lane 0  ├──► GPIO 34
   │ YEL LED ├──220Ω──► GPIO 17  │ IR Lane 1  ├──► GPIO 35
   │ GRN LED ├──220Ω──► GPIO 18  │ IR Lane 2  ├──► GPIO 32
   └─────────┘          │        │ IR Lane 3  ├──► GPIO 33
                        │        └────────────┘
   Lane 3 (West)        │        (VCC → 3.3V, GND → GND)
   ┌─────────┐          │
   │ RED LED ├──220Ω──► GPIO 25
   │ YEL LED ├──220Ω──► GPIO 26
   │ GRN LED ├──220Ω──► GPIO 27
   └─────────┘          │
                        └─────────────────┘
```

---

## IR Sensor Wiring (FC-51 Module)

Each IR sensor module has 3 pins:

| Pin | Connection |
|-----|------------|
| VCC | 3.3V (or 5V if module supports) |
| GND | GND |
| OUT | ESP32 GPIO (see table above) |

- Output is **LOW** when an obstacle (vehicle) is detected
- Output is **HIGH** when the lane is clear
- Adjust the onboard potentiometer for detection distance

---

## Power Considerations

1. **ESP32** can be powered via USB (5V) or VIN pin
2. **MFRC522** operates at **3.3V** — do NOT connect to 5V
3. Each LED draws ~15mA through 220Ω resistor
4. Total LED current: 12 × 15mA = 180mA (worst case if all on)
5. IR sensors: ~20mA each = 80mA total
6. MFRC522: ~26mA during communication
7. **Total estimated current: ~400-500mA** — USB power is sufficient for prototyping
