#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

#define FIRMWARE_VERSION "v1.0.0"
#define WIFI_SSID "TRX4M-ProScale"
#define WIFI_PASS ""

#define PIN_BTN D2
#define PIN_RX_IN D7
#define PIN_LED_MAIN D0
#define PIN_LED_AUX D1
#define PIN_LED 15
#define PIN_RF_EN 3
#define PIN_RF_SEL 14

#define NUM_LEDS_MAIN 4
#define NUM_LEDS_AUX 1

// ==========================================
// LED HELLIGKEITS-PROFILE (0-255)
// ==========================================
#define BRIGHT_MAX       255 // Volle Helligkeit (Blinker, Volllicht, Bremse)
#define BRIGHT_TAIL       50 // Rücklicht (Normales Fahren)
#define BRIGHT_US_MARKER  40 // US-Style Frontblinker (Glimmen)
#define BRIGHT_DIMMED     30 // Abgedimmtes Frontlicht (Bei Abblendlicht oder Parken)

// ==========================================
// BLINKER & STEUERUNG SCHWELLENWERTE
// ==========================================
#define STICK_DEADBAND 50        // Abweichung von 1500, in der nichts passiert (1450 - 1550)
#define STICK_BLINK_THRESH 150   // Ab wie viel Ausschlag der Blinker angeht (>1650 oder <1350)
#define BLINK_INTERVAL_MS 600    // Wechselintervall in Millisekunden
#define ESS_THRESHOLD_PERCENT 90 // ESS-Threshold in Prozent


struct Config
{
  int protocol;
  int ch_steer;
  int ch_throttle;
  int ch_light;
  int ch_aux;
  int ch_blinker_man;
  int ch_hazard;
  int ch_gear;
  bool rev_steer;
  bool rev_throttle;
  bool rev_light;
  bool rev_aux;
  bool rev_blinker_man;
  bool rev_hazard;
  bool rev_gear;
  int trim_steer;
  int mode_blinker;
  int style_blinker;
  int mode_esc;
  bool ess_active;
  int timeout_park;
  int style_region;
  int master_brightness;
  bool enable_soft_fade;
};

// Externe Variablen (werden in main.cpp deklariert)
extern Config cfg;
extern uint16_t rc_channels[14];
extern unsigned long last_rc_time;
extern bool signalValid;

extern bool configMode;
extern volatile bool clientDisconnected;
extern unsigned long letzteBewegung;

extern Adafruit_NeoPixel stripMain;
extern Adafruit_NeoPixel stripAux;

extern bool car_is_driving;
extern bool car_is_reversing;
extern bool car_is_braking;
extern bool car_is_standing;
extern bool car_is_parked;