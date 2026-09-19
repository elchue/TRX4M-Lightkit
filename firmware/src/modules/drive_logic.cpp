#include "drive_logic.h"
#include "globals.h"

// Die globalen Zustände initialisieren
bool car_is_driving = false;
bool car_is_reversing = false;
bool car_is_braking = false;
bool car_is_standing = true;
bool car_is_parked = false;

// Lokale Variablen für die State-Machines
static unsigned long time_stopped = 0;
static bool lock_reverse = false;

void updateDriveLogic(int valThrottle, int valSteering, int valGear) {
  
  // 1. AKTIVITÄTS-CHECK (Für das Parklicht)
  if (valThrottle > 1550 || valThrottle < 1450 || valSteering > 1550 || valSteering < 1450) {
    letzteBewegung = millis(); 
    car_is_parked = false;
  }

  if (cfg.timeout_park > 0 && (millis() - letzteBewegung) > (cfg.timeout_park * 1000UL)) {
    car_is_parked = true;
    car_is_braking = false; 
  }

  // ==========================================
  // MODUS 0: CRAWLER (Drag-Brake)
  // ==========================================
  if (cfg.mode_esc == 0) {
    if (valThrottle > 1550) { // VORWÄRTS
      car_is_driving = true; car_is_reversing = false; car_is_braking = false; car_is_standing = false;
    } 
    else if (valThrottle < 1450) { // RÜCKWÄRTS
      car_is_driving = false; car_is_reversing = true; car_is_braking = false; car_is_standing = false;
    } 
    else { // NEUTRAL
      if (!car_is_standing) {
        car_is_standing = true; car_is_driving = false; car_is_reversing = false; car_is_braking = true;
        time_stopped = millis();
      }
      if (car_is_braking && (millis() - time_stopped > 3000)) {
        car_is_braking = false; // Bremslicht geht nach 3 Sekunden im Stand aus
      }
    }
  }
  
  // ==========================================
  // MODUS 1: STANDARD (Doppelklick-Rückwärts)
  // ==========================================
  else if (cfg.mode_esc == 1) {
    if (valThrottle > 1550) { // VORWÄRTS
      car_is_driving = true; car_is_reversing = false; car_is_braking = false; car_is_standing = false;
      lock_reverse = true; // Rückwärtsgang sperren, nächster Push nach hinten ist Bremse!
    } 
    else if (valThrottle < 1450) { // BREMSE ODER RÜCKWÄRTS?
      car_is_driving = false; car_is_standing = false;
      if (lock_reverse) {
        car_is_braking = true; car_is_reversing = false; // 1. Push: Bremse
      } else {
        car_is_braking = false; car_is_reversing = true; // 2. Push: Rückwärts
      }
    } 
    else { // NEUTRAL
      car_is_driving = false; car_is_reversing = false; car_is_standing = true;
      if (car_is_braking) {
        car_is_braking = false; 
        lock_reverse = false; // Sperre aufheben, wenn Hebel losgelassen wurde!
      }
    }
  }

  // ==========================================
  // MODUS 2: REAL-CAR (Gangschaltung über AUX)
  // ==========================================
  else if (cfg.mode_esc == 2) {
    // Schalter (gear_val): <1300 Rückwärts | 1300-1700 Neutral | >1700 Vorwärts
    bool gear_is_forward = (valGear > 1700);
    bool gear_is_reverse = (valGear < 1300);

    if (valThrottle > 1550) { // GAS GEGEBEN
      car_is_braking = false; car_is_standing = false;
      if (gear_is_forward) { car_is_driving = true; car_is_reversing = false; }
      else if (gear_is_reverse) { car_is_driving = false; car_is_reversing = true; }
      else { car_is_driving = false; car_is_reversing = false; } // Motor heult im Leerlauf auf (kein Licht)
    } 
    else if (valThrottle < 1450) { // BREMSE GETRETEN (Egal welcher Gang!)
      car_is_driving = false; car_is_reversing = false; car_is_standing = false; car_is_braking = true;
    } 
    else { // FUSS VOM GAS
      car_is_driving = false; car_is_reversing = false; car_is_braking = false; car_is_standing = true;
    }
  }
}