/**
 * ir_sensor.cpp — IR sensor array implementation
 */

#include "ir_sensor.h"

// ── Pin arrays (defined here, declared extern in config.h) ──────────────────

const int IR_PINS[NUM_LANES] = {
    IR_SENSOR_LANE0,
    IR_SENSOR_LANE1,
    IR_SENSOR_LANE2,
    IR_SENSOR_LANE3
};

// ── Constructor ─────────────────────────────────────────────────────────────

IRSensorArray::IRSensorArray() {
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        _windowIndex[i] = 0;
        _windowSum[i]   = 0;
        _windowFull[i]  = false;
        memset(_window[i], 0, DENSITY_WINDOW_SIZE);
    }
}

// ── Initialization ──────────────────────────────────────────────────────────

void IRSensorArray::begin() {
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        pinMode(IR_PINS[i], INPUT);
    }
    Serial.println("[IR] IR sensor array initialized.");
}

// ── Update density sliding window ───────────────────────────────────────────

void IRSensorArray::updateDensity() {
    for (uint8_t lane = 0; lane < NUM_LANES; lane++) {
        // IR sensor: LOW = vehicle detected, HIGH = clear
        uint8_t reading = (digitalRead(IR_PINS[lane]) == LOW) ? 1 : 0;

        // Subtract the old value from the running sum
        _windowSum[lane] -= _window[lane][_windowIndex[lane]];

        // Store the new reading
        _window[lane][_windowIndex[lane]] = reading;
        _windowSum[lane] += reading;

        // Advance the circular index
        _windowIndex[lane]++;
        if (_windowIndex[lane] >= DENSITY_WINDOW_SIZE) {
            _windowIndex[lane] = 0;
            _windowFull[lane]  = true;
        }
    }
}

// ── Get density as a percentage ─────────────────────────────────────────────

uint8_t IRSensorArray::getDensityPercent(uint8_t lane) const {
    if (lane >= NUM_LANES) return 0;

    uint8_t count = _windowFull[lane] ? DENSITY_WINDOW_SIZE : _windowIndex[lane];
    if (count == 0) return 0;

    return (uint8_t)((_windowSum[lane] * 100UL) / count);
}

// ── Classify density level ──────────────────────────────────────────────────

DensityLevel IRSensorArray::getDensityLevel(uint8_t lane) const {
    uint8_t pct = getDensityPercent(lane);

    if (pct == 0)                          return DENSITY_NONE;
    if (pct < DENSITY_LOW_THRESHOLD)       return DENSITY_LOW;
    if (pct < DENSITY_HIGH_THRESHOLD)      return DENSITY_MEDIUM;
    return DENSITY_HIGH;
}

// ── Instantaneous vehicle presence ──────────────────────────────────────────

bool IRSensorArray::isVehiclePresent(uint8_t lane) const {
    if (lane >= NUM_LANES) return false;
    return (digitalRead(IR_PINS[lane]) == LOW);
}

// ── Compute adaptive green time based on density ────────────────────────────

unsigned long IRSensorArray::computeGreenTime(uint8_t lane) const {
    uint8_t pct = getDensityPercent(lane);

    // Linear interpolation: MIN_GREEN at 0% density → MAX_GREEN at 100%
    unsigned long greenTime = MIN_GREEN_TIME_MS +
        ((unsigned long)pct * (MAX_GREEN_TIME_MS - MIN_GREEN_TIME_MS)) / 100;

    // Clamp
    if (greenTime < MIN_GREEN_TIME_MS) greenTime = MIN_GREEN_TIME_MS;
    if (greenTime > MAX_GREEN_TIME_MS) greenTime = MAX_GREEN_TIME_MS;

    return greenTime;
}

// ── Get highest-density lane ────────────────────────────────────────────────

uint8_t IRSensorArray::getHighestDensityLane() const {
    uint8_t maxLane = 0;
    uint8_t maxDensity = 0;

    for (uint8_t i = 0; i < NUM_LANES; i++) {
        uint8_t d = getDensityPercent(i);
        if (d > maxDensity) {
            maxDensity = d;
            maxLane = i;
        }
    }
    return maxLane;
}

// ── Debug print ─────────────────────────────────────────────────────────────

void IRSensorArray::printDensityInfo() const {
    extern const char* LANE_NAMES[];
    
    Serial.println("── IR Sensor Density ──");
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        Serial.printf("  %-6s: %3d%% [%s] | Vehicle: %s\n",
            LANE_NAMES[i],
            getDensityPercent(i),
            (getDensityLevel(i) == DENSITY_HIGH)   ? "HIGH  " :
            (getDensityLevel(i) == DENSITY_MEDIUM)  ? "MEDIUM" :
            (getDensityLevel(i) == DENSITY_LOW)     ? "LOW   " : "NONE  ",
            isVehiclePresent(i) ? "YES" : "NO"
        );
    }
}
