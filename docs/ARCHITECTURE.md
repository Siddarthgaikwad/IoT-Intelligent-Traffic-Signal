# System Architecture

## State Machine Diagram

```
                    ┌─────────────────────────────────────────────┐
                    │                                             │
                    ▼                                             │
              ┌──────────┐                                        │
              │          │                                        │
     ───────► │   IDLE   │ ◄── System Boot                       │
              │          │                                        │
              └────┬─────┘                                        │
                   │                                              │
                   │ (after ALL_RED_TIME_MS)                      │
                   ▼                                              │
              ┌──────────────┐                                    │
     ┌───────►│              │                                    │
     │        │ GREEN_PHASE  │──── [RFID Tag] ──────┐             │
     │        │              │                      │             │
     │        └──────┬───────┘                      │             │
     │               │                              │             │
     │               │ (green time expired)         │             │
     │               ▼                              │             │
     │        ┌──────────────┐                      │             │
     │        │              │                      ▼             │
     │        │ YELLOW_PHASE │──── [RFID Tag] ──► ┌───────────┐   │
     │        │              │                    │           │   │
     │        └──────┬───────┘                    │ EMERGENCY │   │
     │               │                            │           │   │
     │               │ (YELLOW_TIME_MS)           └─────┬─────┘   │
     │               ▼                                  │         │
     │        ┌───────────────┐                         │         │
     │        │               │                         │ (15s)   │
     │        │ ALL_RED_PHASE │──── [RFID Tag] ────►    │         │
     │        │               │                         ▼         │
     │        └──────┬────────┘               ┌─────────────────┐ │
     │               │                        │                 │ │
     │               │ (ALL_RED_TIME_MS)      │ EMERGENCY_CLEAR │ │
     │               │                        │                 │ │
     └───────────────┘                        └────────┬────────┘ │
         (next lane selected)                          │          │
                                                       └──────────┘
                                                   (resume normal)
```

## Module Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        main.cpp                                 │
│                   (Application Entry Point)                     │
│                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌──────────────────┐       │
│  │  IRSensor   │  │   RFID      │  │  SignalController │       │
│  │  Array      │  │   Handler   │  │                   │       │
│  │             │  │             │  │  - Red LEDs       │       │
│  │  - 4x IR   │  │  - MFRC522  │  │  - Yellow LEDs    │       │
│  │  - Density  │  │  - Tag DB   │  │  - Green LEDs     │       │
│  │  - Window   │  │  - Events   │  │  - Per-lane ctrl  │       │
│  └──────┬──────┘  └──────┬──────┘  └────────┬──────────┘       │
│         │                │                   │                  │
│         └────────┬───────┘                   │                  │
│                  │                           │                  │
│         ┌────────▼───────────────────────────▼──────┐           │
│         │                                           │           │
│         │       TrafficStateMachine (FSM)           │           │
│         │                                           │           │
│         │  - State transitions                      │           │
│         │  - Density-based green time               │           │
│         │  - Emergency override                     │           │
│         │  - Round-robin + priority scheduling      │           │
│         │                                           │           │
│         └───────────────────────────────────────────┘           │
│                                                                 │
│  ┌─────────────────────────────────────────────────────┐       │
│  │                config.h                              │       │
│  │  - Pin definitions                                   │       │
│  │  - Timing constants                                  │       │
│  │  - Thresholds                                        │       │
│  └─────────────────────────────────────────────────────┘       │
└─────────────────────────────────────────────────────────────────┘
```

## Signal Timing Diagram

### Normal Operation (4 lanes)

```
Lane 0: ██████████████████████████░░░░░░█████████████████████████
         GREEN (density-adjusted)  YEL  ──── RED ────────────────

Lane 1: ████████████████████████████████████████████████████░░░░░
         ──────────── RED ──────────────  GREEN (adj.)     YEL

Lane 2: (similar pattern, shifted)

Lane 3: (similar pattern, shifted)

        ◄─── All-Red gap (2s) ──►
```

### Emergency Override

```
Normal:  Lane 1 GREEN ──► ALL RED (500ms) ──► Lane X EMERGENCY GREEN (15s) ──► YELLOW (3s) ──► Resume
```

## Data Flow

```
IR Sensors ──► updateDensity() ──► Sliding Window ──► getDensityPercent()
                                                           │
                                                           ▼
                                                   computeGreenTime()
                                                           │
                                                           ▼
RFID Reader ──► scan() ──► Tag Lookup ──► EmergencyEvent ──► TrafficFSM
                                                               │
                                                               ▼
                                                       SignalController
                                                           │
                                                           ▼
                                                     LED Outputs
```
