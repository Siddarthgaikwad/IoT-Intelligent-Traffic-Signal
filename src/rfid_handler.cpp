/**
 * rfid_handler.cpp — RFID emergency vehicle detection implementation
 */

#include "rfid_handler.h"

// ── Constructor ─────────────────────────────────────────────────────────────

RFIDHandler::RFIDHandler()
    : _rfid(RFID_SS_PIN, RFID_RST_PIN),
      _tagCount(0),
      _lastEmergencyMs(0)
{
    _event.active       = false;
    _event.priorityLane = 0;
    _event.tagUID       = "";

    for (uint8_t i = 0; i < MAX_EMERGENCY_TAGS; i++) {
        _tags[i].used = false;
    }
}

// ── Initialization ──────────────────────────────────────────────────────────

void RFIDHandler::begin() {
    _rfid.PCD_Init();
    delay(100);

    // Self-test: check firmware version
    byte version = _rfid.PCD_ReadRegister(MFRC522::VersionReg);
    if (version == 0x00 || version == 0xFF) {
        Serial.println("[RFID] WARNING: MFRC522 not detected! Check wiring.");
    } else {
        Serial.printf("[RFID] MFRC522 firmware v0x%02X initialized.\n", version);
    }

    // ── Pre-register some example emergency vehicle tags ──
    // Replace these UIDs with the actual UIDs of your RFID tags.
    registerEmergencyTag("A3 B2 C1 D0", 0);  // Ambulance → North lane
    registerEmergencyTag("11 22 33 44", 1);  // Fire truck → East lane
    registerEmergencyTag("AA BB CC DD", 2);  // Police     → South lane
    registerEmergencyTag("FF EE DD CC", 3);  // VIP escort → West lane

    Serial.printf("[RFID] %d emergency tags registered.\n", _tagCount);
}

// ── Scan for new RFID card ──────────────────────────────────────────────────

void RFIDHandler::scan() {
    // Don't re-trigger during cooldown
    if (_event.active) return;
    if (millis() - _lastEmergencyMs < EMERGENCY_COOLDOWN_MS) return;

    // Check for a new card
    if (!_rfid.PICC_IsNewCardPresent()) return;
    if (!_rfid.PICC_ReadCardSerial())   return;

    // Convert UID to string
    String uid = uidToString(_rfid.uid.uidByte, _rfid.uid.size);
    Serial.printf("[RFID] Card detected: %s\n", uid.c_str());

    // Check if it's a registered emergency tag
    int idx = _findTag(uid);
    if (idx >= 0) {
        _event.active       = true;
        _event.priorityLane = _tags[idx].lane;
        _event.tagUID       = uid;
        _lastEmergencyMs    = millis();

        Serial.printf("[RFID] *** EMERGENCY VEHICLE DETECTED ***\n");
        Serial.printf("[RFID]     Tag: %s → Priority Lane: %d (%s)\n",
            uid.c_str(), _tags[idx].lane,
            (const char*[]){"North", "East", "South", "West"}[_tags[idx].lane]);
    } else {
        Serial.printf("[RFID] Unknown tag: %s (not an emergency vehicle)\n", uid.c_str());
    }

    // Halt the card to prevent repeated reads
    _rfid.PICC_HaltA();
    _rfid.PCD_StopCrypto1();
}

// ── Emergency event accessors ───────────────────────────────────────────────

bool RFIDHandler::hasEmergencyEvent() const {
    return _event.active;
}

EmergencyEvent RFIDHandler::getEmergencyEvent() const {
    return _event;
}

void RFIDHandler::clearEmergencyEvent() {
    _event.active = false;
    _event.tagUID = "";
    Serial.println("[RFID] Emergency event cleared.");
}

// ── Tag registry management ────────────────────────────────────────────────

bool RFIDHandler::registerEmergencyTag(const String& uid, uint8_t lane) {
    if (lane >= NUM_LANES) return false;
    if (_findTag(uid) >= 0) return false;  // already registered

    for (uint8_t i = 0; i < MAX_EMERGENCY_TAGS; i++) {
        if (!_tags[i].used) {
            _tags[i].uid  = uid;
            _tags[i].lane = lane;
            _tags[i].used = true;
            _tagCount++;
            return true;
        }
    }
    Serial.println("[RFID] ERROR: Tag registry full!");
    return false;
}

bool RFIDHandler::removeEmergencyTag(const String& uid) {
    int idx = _findTag(uid);
    if (idx < 0) return false;

    _tags[idx].used = false;
    _tags[idx].uid  = "";
    _tagCount--;
    return true;
}

// ── Helpers ─────────────────────────────────────────────────────────────────

int RFIDHandler::_findTag(const String& uid) const {
    for (uint8_t i = 0; i < MAX_EMERGENCY_TAGS; i++) {
        if (_tags[i].used && _tags[i].uid == uid) {
            return (int)i;
        }
    }
    return -1;
}

String RFIDHandler::uidToString(byte* uid, byte size) {
    String s = "";
    for (byte i = 0; i < size; i++) {
        if (i > 0) s += " ";
        if (uid[i] < 0x10) s += "0";
        s += String(uid[i], HEX);
    }
    s.toUpperCase();
    return s;
}
