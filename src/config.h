/**
 * config.h — Pin definitions and system constants
 * 
 * Adjust these pins to match your wiring.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ════════════════════════════════════════════════════════════════════════════
//  NUMBER OF TRAFFIC LANES / DIRECTIONS
// ════════════════════════════════════════════════════════════════════════════

#define NUM_LANES   4   // North, East, South, West

// ════════════════════════════════════════════════════════════════════════════
//  TRAFFIC SIGNAL LED PINS  (Red, Yellow, Green per lane)
// ════════════════════════════════════════════════════════════════════════════

// Lane 0 — North
#define LANE0_RED_PIN       2
#define LANE0_YELLOW_PIN    4
#define LANE0_GREEN_PIN     5

// Lane 1 — East
#define LANE1_RED_PIN       13
#define LANE1_YELLOW_PIN    14
#define LANE1_GREEN_PIN     15

// Lane 2 — South
#define LANE2_RED_PIN       16
#define LANE2_YELLOW_PIN    17
#define LANE2_GREEN_PIN     18

// Lane 3 — West
#define LANE3_RED_PIN       25
#define LANE3_YELLOW_PIN    26
#define LANE3_GREEN_PIN     27

// Packed arrays for easy iteration (defined in signal_controller.cpp)
extern const int RED_PINS[NUM_LANES];
extern const int YELLOW_PINS[NUM_LANES];
extern const int GREEN_PINS[NUM_LANES];

// ════════════════════════════════════════════════════════════════════════════
//  IR SENSOR PINS (one per lane — digital input)
// ════════════════════════════════════════════════════════════════════════════

#define IR_SENSOR_LANE0     34   // North  (input-only GPIOs are fine)
#define IR_SENSOR_LANE1     35   // East
#define IR_SENSOR_LANE2     32   // South
#define IR_SENSOR_LANE3     33   // West

extern const int IR_PINS[NUM_LANES];

// ════════════════════════════════════════════════════════════════════════════
//  RFID (MFRC522) PINS — SPI interface
// ════════════════════════════════════════════════════════════════════════════

#define RFID_SS_PIN         21   // SDA / SS
#define RFID_RST_PIN        22   // RST
#define RFID_SCK_PIN        SCK  // default SPI clock
#define RFID_MISO_PIN       MISO
#define RFID_MOSI_PIN       MOSI

// ════════════════════════════════════════════════════════════════════════════
//  TIMING CONSTANTS (milliseconds)
// ════════════════════════════════════════════════════════════════════════════

// Base green time when no density data is available (ms)
#define BASE_GREEN_TIME_MS          10000   // 10 seconds

// Minimum and maximum green time (density-adjusted)
#define MIN_GREEN_TIME_MS           5000    // 5 seconds
#define MAX_GREEN_TIME_MS           30000   // 30 seconds

// Yellow transition time
#define YELLOW_TIME_MS              3000    // 3 seconds

// All-red clearance time (safety gap between phases)
#define ALL_RED_TIME_MS             2000    // 2 seconds

// Emergency override green duration
#define EMERGENCY_GREEN_TIME_MS     15000   // 15 seconds

// Emergency cooldown — block re-trigger for this long after an override
#define EMERGENCY_COOLDOWN_MS       5000    // 5 seconds

// ════════════════════════════════════════════════════════════════════════════
//  SAMPLING / POLLING INTERVALS
// ════════════════════════════════════════════════════════════════════════════

#define IR_READ_INTERVAL_MS         200     // read IR sensors every 200 ms
#define RFID_CHECK_INTERVAL_MS      100     // poll RFID reader every 100 ms
#define SERIAL_PRINT_INTERVAL_MS    2000    // debug print every 2 s

// ════════════════════════════════════════════════════════════════════════════
//  VEHICLE DENSITY THRESHOLDS
// ════════════════════════════════════════════════════════════════════════════

// IR sensor returns LOW (0) when a vehicle is detected.
// Density is measured as a percentage of "active" readings over a sliding
// window (see ir_sensor.cpp).

#define DENSITY_LOW_THRESHOLD       25      // <25 % → low traffic
#define DENSITY_HIGH_THRESHOLD      70      // >70 % → heavy traffic

// Window size for density averaging (number of samples)
#define DENSITY_WINDOW_SIZE         20

// ════════════════════════════════════════════════════════════════════════════
//  EMERGENCY RFID TAGS — UIDs of authorized emergency vehicles
// ════════════════════════════════════════════════════════════════════════════

#define MAX_EMERGENCY_TAGS          10

// Lane names (for serial output)
extern const char* LANE_NAMES[NUM_LANES];

#endif // CONFIG_H
