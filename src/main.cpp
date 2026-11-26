/**
 * ============================================================================
 * IoT-Enabled Intelligent Traffic Signal System
 * ============================================================================
 * 
 * Platform:    ESP32 (DevKit V1 / WROOM-32)
 * Language:    Embedded C/C++ (Arduino Framework)
 * Sensors:     IR Sensors (vehicle density), RFID (emergency vehicle detection)
 * Actuators:   Traffic Signal LEDs (Red, Yellow, Green per lane)
 * 
 * Description:
 *   State-machine-based traffic controller that dynamically adjusts signal
 *   timings based on real-time vehicle density from IR sensors. RFID-based
 *   emergency vehicle detection interrupts normal cycles to provide priority
 *   green signals on emergency routes.
 * 
 * Author:      IoT Project Team
 * Date:        Sep 2025 – Jan 2026
 * ============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "config.h"
#include "traffic_state_machine.h"
#include "ir_sensor.h"
#include "rfid_handler.h"
#include "signal_controller.h"

// ── Global Objects ──────────────────────────────────────────────────────────

IRSensorArray     irSensors;
RFIDHandler       rfidHandler;
SignalController  signalCtrl;
TrafficStateMachine trafficFSM;

// ── Timing Variables ────────────────────────────────────────────────────────

unsigned long lastDensityReadMs  = 0;
unsigned long lastRFIDCheckMs    = 0;
unsigned long lastSerialPrintMs  = 0;

// ── Setup ───────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("============================================");
    Serial.println("  IoT Intelligent Traffic Signal System");
    Serial.println("  ESP32 | IR Sensors | RFID | State Machine");
    Serial.println("============================================");
    Serial.println();

    // Initialize SPI bus for RFID
    SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);

    // Initialize subsystems
    irSensors.begin();
    rfidHandler.begin();
    signalCtrl.begin();
    trafficFSM.begin(&irSensors, &rfidHandler, &signalCtrl);

    Serial.println("[INIT] All subsystems initialized successfully.");
    Serial.println("[INIT] Traffic control active.\n");
}

// ── Main Loop ───────────────────────────────────────────────────────────────

void loop() {
    unsigned long now = millis();

    // ── 1. Read IR sensor density at regular intervals ──
    if (now - lastDensityReadMs >= IR_READ_INTERVAL_MS) {
        lastDensityReadMs = now;
        irSensors.updateDensity();
    }

    // ── 2. Check RFID for emergency vehicles ──
    if (now - lastRFIDCheckMs >= RFID_CHECK_INTERVAL_MS) {
        lastRFIDCheckMs = now;
        rfidHandler.scan();
    }

    // ── 3. Run the traffic state machine ──
    trafficFSM.update();

    // ── 4. Debug output ──
    if (now - lastSerialPrintMs >= SERIAL_PRINT_INTERVAL_MS) {
        lastSerialPrintMs = now;
        trafficFSM.printStatus();
    }
}
