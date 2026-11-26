/**
 * rfid_handler.h — RFID-based emergency vehicle detection
 * 
 * Uses MFRC522 RFID reader to detect pre-registered emergency vehicle tags.
 * When a registered tag is scanned, an emergency override is triggered.
 */

#ifndef RFID_HANDLER_H
#define RFID_HANDLER_H

#include <Arduino.h>
#include <MFRC522.h>
#include "config.h"

/**
 * Emergency event info — which lane should get priority.
 */
struct EmergencyEvent {
    bool     active;        // true if an emergency override is pending
    uint8_t  priorityLane;  // lane that should receive the green signal
    String   tagUID;        // UID string of the detected tag
};

/**
 * RFIDHandler
 * 
 * Manages the MFRC522 RFID reader, maintains a table of authorized
 * emergency vehicle UIDs, and signals emergency events to the state machine.
 */
class RFIDHandler {
public:
    RFIDHandler();

    /** Initialize the RFID reader hardware. */
    void begin();

    /** Poll the reader for a new card. Call periodically. */
    void scan();

    /** Check if an emergency event is pending. */
    bool hasEmergencyEvent() const;

    /** Get the current emergency event details. */
    EmergencyEvent getEmergencyEvent() const;

    /** Acknowledge / clear the current emergency event. */
    void clearEmergencyEvent();

    /** Register an emergency vehicle UID → lane mapping. */
    bool registerEmergencyTag(const String& uid, uint8_t lane);

    /** Remove an emergency vehicle UID from the registry. */
    bool removeEmergencyTag(const String& uid);

    /** Convert raw UID bytes to a hex string. */
    static String uidToString(byte* uid, byte size);

private:
    MFRC522  _rfid;
    
    // Emergency tag registry
    struct TagEntry {
        String  uid;
        uint8_t lane;
        bool    used;
    };
    TagEntry _tags[MAX_EMERGENCY_TAGS];
    uint8_t  _tagCount;

    // Current emergency event
    EmergencyEvent _event;

    // Cooldown tracking
    unsigned long _lastEmergencyMs;

    /** Look up a UID in the registry. Returns index or -1. */
    int _findTag(const String& uid) const;
};

#endif // RFID_HANDLER_H
