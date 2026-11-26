/**
 * ir_sensor.h — IR sensor array for vehicle density measurement
 */

#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include <Arduino.h>
#include "config.h"

/**
 * Density level classification.
 */
enum DensityLevel {
    DENSITY_NONE,       // No vehicles detected
    DENSITY_LOW,        // Light traffic
    DENSITY_MEDIUM,     // Moderate traffic
    DENSITY_HIGH        // Heavy traffic / congestion
};

/**
 * IRSensorArray
 * 
 * Manages IR sensors across all lanes. Each sensor provides a digital
 * signal: LOW when a vehicle is present, HIGH when the lane is clear.
 * 
 * Vehicle density is estimated using a sliding window of recent readings.
 */
class IRSensorArray {
public:
    IRSensorArray();

    /** Initialize all IR sensor pins. */
    void begin();

    /** Sample all sensors and update density windows. Call periodically. */
    void updateDensity();

    /** Get the current density percentage (0–100) for a lane. */
    uint8_t getDensityPercent(uint8_t lane) const;

    /** Get the classified density level for a lane. */
    DensityLevel getDensityLevel(uint8_t lane) const;

    /** Get instantaneous reading: true if vehicle is present on the lane. */
    bool isVehiclePresent(uint8_t lane) const;

    /** Compute dynamic green time (ms) based on density for a lane. */
    unsigned long computeGreenTime(uint8_t lane) const;

    /** Get the lane with the highest density. */
    uint8_t getHighestDensityLane() const;

    /** Print density info for all lanes (debug). */
    void printDensityInfo() const;

private:
    // Sliding window buffer for each lane
    uint8_t _window[NUM_LANES][DENSITY_WINDOW_SIZE];
    uint8_t _windowIndex[NUM_LANES];
    uint16_t _windowSum[NUM_LANES];
    bool    _windowFull[NUM_LANES];
};

#endif // IR_SENSOR_H
