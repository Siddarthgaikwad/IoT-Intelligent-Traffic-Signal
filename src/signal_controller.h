/**
 * signal_controller.h — Traffic signal LED controller
 * 
 * Controls the Red/Yellow/Green LEDs for each lane.
 */

#ifndef SIGNAL_CONTROLLER_H
#define SIGNAL_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

/**
 * Signal state for a single lane.
 */
enum SignalState {
    SIGNAL_RED,
    SIGNAL_YELLOW,
    SIGNAL_GREEN,
    SIGNAL_OFF         // All LEDs off (used during initialization or error)
};

/**
 * SignalController
 * 
 * Low-level driver that sets LED states for each lane.
 */
class SignalController {
public:
    SignalController();

    /** Initialize all LED pins as OUTPUT and set all lanes to RED. */
    void begin();

    /** Set the signal state for a specific lane. */
    void setLaneSignal(uint8_t lane, SignalState state);

    /** Set all lanes to RED (safe state). */
    void allRed();

    /** Set all lanes to OFF. */
    void allOff();

    /** Flash all yellows (used for maintenance / warning mode). */
    void flashYellow();

    /** Get the current signal state of a lane. */
    SignalState getLaneSignal(uint8_t lane) const;

    /** Get a human-readable string for a signal state. */
    static const char* signalStateToString(SignalState state);

    /** Print current signal states for all lanes. */
    void printSignalStates() const;

private:
    SignalState _laneStates[NUM_LANES];

    /** Turn a single LED on or off. */
    void _setLED(int pin, bool on);
};

#endif // SIGNAL_CONTROLLER_H
