/**
 * signal_controller.cpp — Traffic signal LED controller implementation
 */

#include "signal_controller.h"

// ── Pin arrays (defined here, declared extern in config.h) ──────────────────

const int RED_PINS[NUM_LANES] = {
    LANE0_RED_PIN, LANE1_RED_PIN, LANE2_RED_PIN, LANE3_RED_PIN
};

const int YELLOW_PINS[NUM_LANES] = {
    LANE0_YELLOW_PIN, LANE1_YELLOW_PIN, LANE2_YELLOW_PIN, LANE3_YELLOW_PIN
};

const int GREEN_PINS[NUM_LANES] = {
    LANE0_GREEN_PIN, LANE1_GREEN_PIN, LANE2_GREEN_PIN, LANE3_GREEN_PIN
};

// Lane names for debug output
const char* LANE_NAMES[NUM_LANES] = {
    "North", "East", "South", "West"
};

// ── Constructor ─────────────────────────────────────────────────────────────

SignalController::SignalController() {
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        _laneStates[i] = SIGNAL_OFF;
    }
}

// ── Initialization ──────────────────────────────────────────────────────────

void SignalController::begin() {
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        pinMode(RED_PINS[i],    OUTPUT);
        pinMode(YELLOW_PINS[i], OUTPUT);
        pinMode(GREEN_PINS[i],  OUTPUT);
    }

    // Start with all lanes on RED (safe default)
    allRed();
    Serial.println("[SIGNAL] Traffic signal LEDs initialized — all RED.");
}

// ── Set signal for one lane ─────────────────────────────────────────────────

void SignalController::setLaneSignal(uint8_t lane, SignalState state) {
    if (lane >= NUM_LANES) return;

    // Turn off all LEDs for this lane first
    _setLED(RED_PINS[lane],    false);
    _setLED(YELLOW_PINS[lane], false);
    _setLED(GREEN_PINS[lane],  false);

    // Turn on the appropriate LED
    switch (state) {
        case SIGNAL_RED:
            _setLED(RED_PINS[lane], true);
            break;
        case SIGNAL_YELLOW:
            _setLED(YELLOW_PINS[lane], true);
            break;
        case SIGNAL_GREEN:
            _setLED(GREEN_PINS[lane], true);
            break;
        case SIGNAL_OFF:
        default:
            // All off — already handled above
            break;
    }

    _laneStates[lane] = state;
}

// ── Bulk operations ─────────────────────────────────────────────────────────

void SignalController::allRed() {
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        setLaneSignal(i, SIGNAL_RED);
    }
}

void SignalController::allOff() {
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        setLaneSignal(i, SIGNAL_OFF);
    }
}

void SignalController::flashYellow() {
    static bool on = false;
    on = !on;
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        _setLED(RED_PINS[i],    false);
        _setLED(GREEN_PINS[i],  false);
        _setLED(YELLOW_PINS[i], on);
        _laneStates[i] = on ? SIGNAL_YELLOW : SIGNAL_OFF;
    }
}

// ── Accessors ───────────────────────────────────────────────────────────────

SignalState SignalController::getLaneSignal(uint8_t lane) const {
    if (lane >= NUM_LANES) return SIGNAL_OFF;
    return _laneStates[lane];
}

const char* SignalController::signalStateToString(SignalState state) {
    switch (state) {
        case SIGNAL_RED:    return "RED";
        case SIGNAL_YELLOW: return "YELLOW";
        case SIGNAL_GREEN:  return "GREEN";
        case SIGNAL_OFF:    return "OFF";
        default:            return "UNKNOWN";
    }
}

void SignalController::printSignalStates() const {
    Serial.println("── Signal States ──");
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        Serial.printf("  %-6s: %s\n", LANE_NAMES[i],
            signalStateToString(_laneStates[i]));
    }
}

// ── Private helpers ─────────────────────────────────────────────────────────

void SignalController::_setLED(int pin, bool on) {
    digitalWrite(pin, on ? HIGH : LOW);
}
