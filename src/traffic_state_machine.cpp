/**
 * traffic_state_machine.cpp — Core traffic control state machine implementation
 * 
 * ┌──────┐    ┌───────────┐    ┌──────────────┐    ┌───────────────┐
 * │ IDLE │───►│  GREEN    │───►│   YELLOW     │───►│   ALL_RED     │──┐
 * └──────┘    │  PHASE    │    │   PHASE      │    │   PHASE       │  │
 *             └─────┬─────┘    └──────────────┘    └───────────────┘  │
 *                   │                                      │          │
 *              [RFID detected at any point]                │          │
 *                   │                                      ▼          │
 *                   ▼                              (next lane green)──┘
 *             ┌───────────┐    ┌──────────────┐
 *             │ EMERGENCY │───►│  EMERGENCY   │───► (resume normal)
 *             │           │    │  CLEAR       │
 *             └───────────┘    └──────────────┘
 */

#include "traffic_state_machine.h"

// ── Constructor ─────────────────────────────────────────────────────────────

TrafficStateMachine::TrafficStateMachine()
    : _sensors(nullptr),
      _rfid(nullptr),
      _signals(nullptr),
      _state(STATE_IDLE),
      _preEmergencyState(STATE_IDLE),
      _activeLane(0),
      _emergencyLane(0),
      _phaseStartMs(0),
      _currentGreenDuration(BASE_GREEN_TIME_MS),
      _cycleCount(0)
{}

// ── Initialization ──────────────────────────────────────────────────────────

void TrafficStateMachine::begin(IRSensorArray* sensors, RFIDHandler* rfid,
                                  SignalController* signals) {
    _sensors = sensors;
    _rfid    = rfid;
    _signals = signals;

    _state        = STATE_IDLE;
    _activeLane   = 0;
    _phaseStartMs = millis();
    _cycleCount   = 0;

    Serial.println("[FSM] Traffic state machine initialized.");
}

// ── Main update (called every loop iteration) ───────────────────────────────

void TrafficStateMachine::update() {
    if (!_sensors || !_rfid || !_signals) return;

    switch (_state) {
        case STATE_IDLE:            _handleIdle();            break;
        case STATE_GREEN_PHASE:     _handleGreenPhase();      break;
        case STATE_YELLOW_PHASE:    _handleYellowPhase();     break;
        case STATE_ALL_RED_PHASE:   _handleAllRedPhase();     break;
        case STATE_EMERGENCY:       _handleEmergency();       break;
        case STATE_EMERGENCY_CLEAR: _handleEmergencyClear();  break;
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  STATE HANDLERS
// ════════════════════════════════════════════════════════════════════════════

void TrafficStateMachine::_handleIdle() {
    // Brief all-red startup, then start the first green phase
    if (millis() - _phaseStartMs >= ALL_RED_TIME_MS) {
        _signals->allRed();
        _startGreenPhase(0);  // Start with Lane 0 (North)
    }
}

void TrafficStateMachine::_handleGreenPhase() {
    // Check for emergency interruption
    if (_checkEmergency()) return;

    // Wait for green duration to expire
    if (millis() - _phaseStartMs >= _currentGreenDuration) {
        // Transition to yellow
        _transitionTo(STATE_YELLOW_PHASE);
        _signals->setLaneSignal(_activeLane, SIGNAL_YELLOW);
        Serial.printf("[FSM] Lane %d (%s): GREEN → YELLOW\n",
            _activeLane, LANE_NAMES[_activeLane]);
    }
}

void TrafficStateMachine::_handleYellowPhase() {
    // Check for emergency interruption
    if (_checkEmergency()) return;

    if (millis() - _phaseStartMs >= YELLOW_TIME_MS) {
        // Transition to all-red clearance
        _transitionTo(STATE_ALL_RED_PHASE);
        _signals->allRed();
        Serial.printf("[FSM] Lane %d (%s): YELLOW → ALL RED\n",
            _activeLane, LANE_NAMES[_activeLane]);
    }
}

void TrafficStateMachine::_handleAllRedPhase() {
    // Check for emergency interruption
    if (_checkEmergency()) return;

    if (millis() - _phaseStartMs >= ALL_RED_TIME_MS) {
        // Select next lane and start its green phase
        uint8_t nextLane = _selectNextLane();
        _startGreenPhase(nextLane);
        _cycleCount++;
    }
}

void TrafficStateMachine::_handleEmergency() {
    // Emergency green is active for the priority lane
    if (millis() - _phaseStartMs >= EMERGENCY_GREEN_TIME_MS) {
        // Start emergency clearance (yellow on the emergency lane)
        _transitionTo(STATE_EMERGENCY_CLEAR);
        _signals->setLaneSignal(_emergencyLane, SIGNAL_YELLOW);
        Serial.printf("[FSM] EMERGENCY: Lane %d (%s) → YELLOW (clearing)\n",
            _emergencyLane, LANE_NAMES[_emergencyLane]);
    }
}

void TrafficStateMachine::_handleEmergencyClear() {
    if (millis() - _phaseStartMs >= YELLOW_TIME_MS) {
        // Clear the emergency event
        _rfid->clearEmergencyEvent();

        // Resume normal operation: all-red then continue cycle
        _signals->allRed();
        Serial.println("[FSM] EMERGENCY CLEAR: Resuming normal cycle.");

        // Resume from the lane that was active before the emergency
        _startGreenPhase(_activeLane);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  TRANSITION HELPERS
// ════════════════════════════════════════════════════════════════════════════

void TrafficStateMachine::_transitionTo(TrafficState newState) {
    _state        = newState;
    _phaseStartMs = millis();
}

void TrafficStateMachine::_startGreenPhase(uint8_t lane) {
    _activeLane = lane;

    // Compute dynamic green time based on vehicle density
    _currentGreenDuration = _sensors->computeGreenTime(lane);

    // Set signals: active lane → GREEN, all others → RED
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        if (i == lane) {
            _signals->setLaneSignal(i, SIGNAL_GREEN);
        } else {
            _signals->setLaneSignal(i, SIGNAL_RED);
        }
    }

    _transitionTo(STATE_GREEN_PHASE);

    Serial.printf("[FSM] Lane %d (%s): GREEN for %lu ms (density: %d%%)\n",
        lane, LANE_NAMES[lane],
        _currentGreenDuration,
        _sensors->getDensityPercent(lane));
}

uint8_t TrafficStateMachine::_selectNextLane() const {
    // Strategy: round-robin, but if any lane has HIGH density, prioritize it.
    
    // First, check if any non-active lane has high density
    uint8_t highestLane    = 0;
    uint8_t highestDensity = 0;
    
    for (uint8_t i = 0; i < NUM_LANES; i++) {
        if (i == _activeLane) continue;  // skip current lane
        uint8_t d = _sensors->getDensityPercent(i);
        if (d > highestDensity) {
            highestDensity = d;
            highestLane    = i;
        }
    }

    // If a lane has high density, prioritize it
    if (highestDensity >= DENSITY_HIGH_THRESHOLD) {
        return highestLane;
    }

    // Otherwise, simple round-robin
    return (_activeLane + 1) % NUM_LANES;
}

bool TrafficStateMachine::_checkEmergency() {
    if (!_rfid->hasEmergencyEvent()) return false;

    EmergencyEvent evt = _rfid->getEmergencyEvent();

    Serial.println("[FSM] ╔══════════════════════════════════════════╗");
    Serial.println("[FSM] ║     🚨 EMERGENCY OVERRIDE ACTIVATED 🚨    ║");
    Serial.printf( "[FSM] ║  Tag: %-34s ║\n", evt.tagUID.c_str());
    Serial.printf( "[FSM] ║  Priority Lane: %d (%s)                  ║\n",
        evt.priorityLane,
        LANE_NAMES[evt.priorityLane]);
    Serial.println("[FSM] ╚══════════════════════════════════════════╝");

    _preEmergencyState = _state;
    _emergencyLane     = evt.priorityLane;

    // All lanes → RED, then priority lane → GREEN
    _signals->allRed();
    delay(500);  // brief all-red gap for safety
    _signals->setLaneSignal(_emergencyLane, SIGNAL_GREEN);

    _transitionTo(STATE_EMERGENCY);
    return true;
}

// ════════════════════════════════════════════════════════════════════════════
//  STATUS / DEBUG
// ════════════════════════════════════════════════════════════════════════════

TrafficState TrafficStateMachine::getState() const {
    return _state;
}

const char* TrafficStateMachine::stateToString(TrafficState state) {
    switch (state) {
        case STATE_IDLE:            return "IDLE";
        case STATE_GREEN_PHASE:     return "GREEN_PHASE";
        case STATE_YELLOW_PHASE:    return "YELLOW_PHASE";
        case STATE_ALL_RED_PHASE:   return "ALL_RED_PHASE";
        case STATE_EMERGENCY:       return "EMERGENCY";
        case STATE_EMERGENCY_CLEAR: return "EMERGENCY_CLEAR";
        default:                    return "UNKNOWN";
    }
}

void TrafficStateMachine::printStatus() const {
    Serial.println("╔══════════════════════════════════════════════╗");
    Serial.println("║       Traffic Signal System — Status         ║");
    Serial.println("╠══════════════════════════════════════════════╣");
    Serial.printf( "║  State:        %-30s ║\n", stateToString(_state));
    Serial.printf( "║  Active Lane:  %d (%s)                       ║\n",
        _activeLane, LANE_NAMES[_activeLane]);
    Serial.printf( "║  Green Time:   %lu ms                        ║\n",
        _currentGreenDuration);
    Serial.printf( "║  Phase Elapsed: %lu ms                       ║\n",
        millis() - _phaseStartMs);
    Serial.printf( "║  Total Cycles: %lu                           ║\n",
        (unsigned long)_cycleCount);
    Serial.println("╚══════════════════════════════════════════════╝");
    
    _sensors->printDensityInfo();
    _signals->printSignalStates();
    Serial.println();
}
