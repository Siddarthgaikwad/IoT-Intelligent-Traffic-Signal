/**
 * traffic_state_machine.h — Core state machine for traffic control
 * 
 * States:
 *   IDLE            → System startup / waiting
 *   GREEN_PHASE     → Active lane has green signal
 *   YELLOW_PHASE    → Transition: active lane yellow before switching
 *   ALL_RED_PHASE   → Safety clearance: all lanes red
 *   EMERGENCY       → Emergency vehicle override: priority lane gets green
 *   EMERGENCY_CLEAR → Transition back from emergency to normal cycle
 * 
 * Normal cycle: rotates through lanes, adjusting green time by density.
 * Emergency: RFID detection → immediate override → priority green → resume.
 */

#ifndef TRAFFIC_STATE_MACHINE_H
#define TRAFFIC_STATE_MACHINE_H

#include <Arduino.h>
#include "config.h"
#include "ir_sensor.h"
#include "rfid_handler.h"
#include "signal_controller.h"

/**
 * Traffic FSM states
 */
enum TrafficState {
    STATE_IDLE,
    STATE_GREEN_PHASE,
    STATE_YELLOW_PHASE,
    STATE_ALL_RED_PHASE,
    STATE_EMERGENCY,
    STATE_EMERGENCY_CLEAR
};

/**
 * TrafficStateMachine
 * 
 * Orchestrates the full traffic control logic using a state machine pattern.
 */
class TrafficStateMachine {
public:
    TrafficStateMachine();

    /** Initialize with references to subsystems. */
    void begin(IRSensorArray* sensors, RFIDHandler* rfid, SignalController* signals);

    /** Run one tick of the state machine. Call in loop(). */
    void update();

    /** Get the current state. */
    TrafficState getState() const;

    /** Get human-readable state name. */
    static const char* stateToString(TrafficState state);

    /** Print full status (state, active lane, timings, density). */
    void printStatus() const;

private:
    // Subsystem references
    IRSensorArray*    _sensors;
    RFIDHandler*      _rfid;
    SignalController* _signals;

    // State machine variables
    TrafficState  _state;
    TrafficState  _preEmergencyState;   // state to return to after emergency
    uint8_t       _activeLane;          // current lane with green/yellow
    uint8_t       _emergencyLane;       // lane with emergency priority
    unsigned long _phaseStartMs;        // when the current phase started
    unsigned long _currentGreenDuration; // computed green time for active lane
    uint32_t      _cycleCount;          // total completed cycles

    // ── State handlers ──
    void _handleIdle();
    void _handleGreenPhase();
    void _handleYellowPhase();
    void _handleAllRedPhase();
    void _handleEmergency();
    void _handleEmergencyClear();

    // ── Transitions ──
    void _transitionTo(TrafficState newState);
    void _startGreenPhase(uint8_t lane);

    /** Select next lane (round-robin with density priority). */
    uint8_t _selectNextLane() const;

    /** Check for emergency and handle if detected. Returns true if emergency started. */
    bool _checkEmergency();
};

#endif // TRAFFIC_STATE_MACHINE_H
