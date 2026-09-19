#include "led_output.h"
#include "globals.h"
#include <math.h> // Für die weichen Fade-Kurven

// Externe Zustände aus der drive_logic
extern bool car_is_driving;
extern bool car_is_reversing;
extern bool car_is_braking;
extern bool car_is_standing;
extern bool car_is_parked;

// ==========================================
// FADING HELPER
// ==========================================
uint8_t fadeTowards(uint8_t current, uint8_t target, uint8_t step) {
  if (current < target) {
    if (current + step > target) return target;
    return current + step;
  } else if (current > target) {
    if (current < step) return 0;
    if (current - step < target) return target;
    return current - step;
  }
  return current;
}

// ==========================================
// 1. HELFER-FUNKTION: BLINKER ANIMATION
// ==========================================
uint8_t getBlinkerPWM(bool active, int style, unsigned long cycle_pos)
{
  if (!active) return 0;

  if (style == 0) { // Classic
    return (cycle_pos < BLINK_INTERVAL_MS) ? BRIGHT_MAX : 0;
  }
  else if (style == 1) { // Mazda (Soft Off)
    if (cycle_pos < BLINK_INTERVAL_MS) {
      return BRIGHT_MAX;
    } else {
      unsigned long off_pos = cycle_pos - BLINK_INTERVAL_MS;
      float fade_out_ms = BLINK_INTERVAL_MS * 0.6;
      if (off_pos < fade_out_ms) {
        float p = off_pos / fade_out_ms;
        return (uint8_t)(BRIGHT_MAX * (1.0 - p));
      }
      return 0;
    }
  }
  else if (style == 2) { // BMW (Soft-On)
    if (cycle_pos < BLINK_INTERVAL_MS) {
      float p = (float)cycle_pos / BLINK_INTERVAL_MS; 
      float curve = p * p; 
      return (uint8_t)(BRIGHT_MAX * curve);
    }
    return 0;
  }
  else if (style == 3) { // Retro (Glühlampen-Fade)
    const float heat_up_ms = 80.0;
    const float cool_down_ms = 150.0;
    if (cycle_pos < BLINK_INTERVAL_MS) {
      if (cycle_pos < heat_up_ms) {
        float p = cycle_pos / heat_up_ms;
        return (uint8_t)(BRIGHT_MAX * (1.0 - pow(1.0 - p, 3)));
      }
      return BRIGHT_MAX;
    } else {
      unsigned long off_pos = cycle_pos - BLINK_INTERVAL_MS;
      if (off_pos < cool_down_ms) {
        float p = off_pos / cool_down_ms;
        return (uint8_t)(BRIGHT_MAX * pow(1.0 - p, 2));
      }
      return 0;
    }
  }
  return (cycle_pos < BLINK_INTERVAL_MS) ? BRIGHT_MAX : 0;
}

// ==========================================
// 2. INITIALISIERUNG
// ==========================================
void initLEDs()
{
  stripMain.begin();
  stripAux.begin();

  int mb = (cfg.master_brightness == 0) ? 100 : cfg.master_brightness;
  uint8_t mapped_bright = map(mb, 0, 100, 0, 255);

  stripMain.setBrightness(mapped_bright);
  stripAux.setBrightness(mapped_bright);

  stripMain.clear();
  stripAux.clear();
  stripMain.show();
  stripAux.show();
}

// ==========================================
// 3. HAUPT-FUNKTION: HARDWARE MAPPING
// ==========================================
void updateLEDs(int valThrottle, int valSteering, int valLight, int valAux, int valBlinkerMan, int valHazard)
{
  // STATISCHE VARIABLEN FÜR SOFT-FADING
  static uint8_t cur_front_g_left = 0;
  static uint8_t cur_front_g_right = 0;
  static uint8_t cur_front_b = 0;
  static uint8_t cur_rear_b_left = 0;
  static uint8_t cur_rear_b_right = 0;
  static uint8_t cur_aux_b = 0;

  const uint8_t FADE_STEP = 6; 

  // ==========================================
  // 1. ESS-LOGIK (Adaptive Brake Light)
  // ==========================================
  static bool ess_was_hard_braking = false; 
  static bool ess_hazard_active = false;
  static unsigned long ess_stand_timer = 0;

  if (cfg.ess_active) {
    const int ess_threshold = (500 * ESS_THRESHOLD_PERCENT) / 100;
    bool is_hard_brake = (car_is_braking && (abs(valThrottle - 1500) > ess_threshold));
    
    if (is_hard_brake) {
      ess_was_hard_braking = true;
      ess_hazard_active = false;
      ess_stand_timer = millis();
    }
    else if (car_is_braking && ess_was_hard_braking) {
      ess_stand_timer = millis();
    }
    else if (ess_was_hard_braking && car_is_standing) {
      if (millis() - ess_stand_timer > 1000) {
        ess_hazard_active = true;
        ess_was_hard_braking = false; 
      }
    }
    if (car_is_driving || car_is_reversing) {
      ess_hazard_active = false;
      ess_was_hard_braking = false;
    }
  } else {
    ess_hazard_active = false;
    ess_was_hard_braking = false;
  }

  // ==========================================
  // 2. BLINKER TRIGGER
  // ==========================================
  bool blink_left_active = false;
  bool blink_right_active = false;

  if (cfg.mode_blinker == 0) {
    if (valSteering < (1500 - STICK_BLINK_THRESH)) blink_left_active = true;
    if (valSteering > (1500 + STICK_BLINK_THRESH)) blink_right_active = true;
  }
  else if (cfg.mode_blinker == 1 || cfg.mode_blinker == 2) {
    static int blink_state = 0;
    static int last_blk_man = 1500;
    static int last_steer = 1500;

    bool steer_left = (valSteering < (1500 - STICK_BLINK_THRESH));
    bool steer_right = (valSteering > (1500 + STICK_BLINK_THRESH));
    bool steer_center = (valSteering > 1400 && valSteering < 1600);
    bool trigger_left = false;
    bool trigger_right = false;

    if (cfg.mode_blinker == 1 && car_is_standing) {
      if (steer_left && !(last_steer < (1500 - STICK_BLINK_THRESH))) trigger_left = true;
      if (steer_right && !(last_steer > (1500 + STICK_BLINK_THRESH))) trigger_right = true;
    }
    if (cfg.mode_blinker == 2 && cfg.ch_blinker_man > 0) {
      if (valBlinkerMan < 1300 && last_blk_man >= 1300) trigger_left = true;
      if (valBlinkerMan > 1700 && last_blk_man <= 1700) trigger_right = true;
    }
    last_steer = valSteering;
    last_blk_man = valBlinkerMan;

    if (trigger_left) { blink_state = (blink_state == 1 || blink_state == 2) ? 0 : 1; }
    if (trigger_right) { blink_state = (blink_state == 3 || blink_state == 4) ? 0 : 3; }

    if (blink_state == 1) {
      if (steer_left && !car_is_standing) blink_state = 2;
      else if (steer_right) blink_state = 0;
    } else if (blink_state == 2) {
      if (steer_center) blink_state = 0;
    } else if (blink_state == 3) {
      if (steer_right && !car_is_standing) blink_state = 4;
      else if (steer_left) blink_state = 0;
    } else if (blink_state == 4) {
      if (steer_center) blink_state = 0;
    }

    blink_left_active = (blink_state == 1 || blink_state == 2);
    blink_right_active = (blink_state == 3 || blink_state == 4);
  }

  if ((cfg.ch_hazard > 0 && valHazard > 1500) || ess_hazard_active) {
    blink_left_active = true;
    blink_right_active = true;
  }

  bool any_blinker_active = (blink_left_active || blink_right_active);
  static unsigned long blink_start_time = 0;
  static bool was_active_last_frame = false;

  if (any_blinker_active && !was_active_last_frame) blink_start_time = millis();
  was_active_last_frame = any_blinker_active;

  unsigned long cycle_pos = any_blinker_active ? ((millis() - blink_start_time) % (BLINK_INTERVAL_MS * 2)) : 0;
  uint8_t pwm_left = getBlinkerPWM(blink_left_active, cfg.style_blinker, cycle_pos);
  uint8_t pwm_right = getBlinkerPWM(blink_right_active, cfg.style_blinker, cycle_pos);

  // ==========================================
  // 3. LICHTSTEUERUNG (ZIELWERTE SETZEN)
  // ==========================================
  bool tfl_on = false;
  bool abl_on = false;
  bool aux_on = false;

  if (cfg.ch_light > 0) {
    if (valLight > 1300) tfl_on = true; 
    if (valLight > 1700) abl_on = true; 
  } else {
    tfl_on = true; abl_on = true;
  }
  if (cfg.ch_aux > 0 && valAux > 1500) aux_on = true;

  uint8_t target_front_g_left = 0;
  uint8_t target_front_g_right = 0;
  uint8_t target_front_b = 0;
  uint8_t target_rear_b_left = 0;
  uint8_t target_rear_b_right = 0;
  uint8_t target_aux_b = aux_on ? BRIGHT_MAX : 0;

  // Smart Side-Parking (Einseitiges Parklicht)
  bool park_left_active = true;
  bool park_right_active = true;
  if (car_is_parked) {
    if (valBlinkerMan < 1200) park_right_active = false;
    if (valBlinkerMan > 1800) park_left_active = false;
  }

  // --- ZIELWERTE FRONT ---
  if (car_is_parked && tfl_on) {
    target_front_g_left = park_left_active ? BRIGHT_DIMMED : 0;
    target_front_g_right = park_right_active ? BRIGHT_DIMMED : 0;
    target_front_b = 0; 
  } 
  else if (tfl_on) {
    uint8_t base_tfl = abl_on ? BRIGHT_DIMMED : BRIGHT_MAX;

    // Zielwert: TFL ist abgedimmt, wenn der Blinker an ist, sonst auf Grundhelligkeit
    target_front_g_left = blink_left_active ? BRIGHT_DIMMED : base_tfl;
    target_front_g_right = blink_right_active ? BRIGHT_DIMMED : base_tfl;

    // HÄRTE-EINGRIFF: Wenn der Blinker angeht, dimmt das TFL SOFORT ab (ohne Fade)
    // Wenn der Blinker aus ist, übernimmt Block 5 das weiche Hochfaden
    if (blink_left_active && cur_front_g_left > BRIGHT_DIMMED) cur_front_g_left = BRIGHT_DIMMED;
    if (blink_right_active && cur_front_g_right > BRIGHT_DIMMED) cur_front_g_right = BRIGHT_DIMMED;

    target_front_b = abl_on ? BRIGHT_MAX : 0;
  }

  // --- ZIELWERTE HECK ---
  if (car_is_parked && tfl_on) {
    target_rear_b_left = park_left_active ? BRIGHT_DIMMED : 0;
    target_rear_b_right = park_right_active ? BRIGHT_DIMMED : 0;
  } 
  else if (car_is_braking) {
    uint8_t brake_val = BRIGHT_MAX;
    if (cfg.ess_active && ess_was_hard_braking) {
      brake_val = ((millis() % 100) < 50) ? BRIGHT_MAX : 0; 
    }
    target_rear_b_left = brake_val;
    target_rear_b_right = brake_val;

    // Bremslicht überschreibt Fading sofort!
    cur_rear_b_left = target_rear_b_left;
    cur_rear_b_right = target_rear_b_right;
  } 
  else if (tfl_on) {
    target_rear_b_left = BRIGHT_TAIL;
    target_rear_b_right = BRIGHT_TAIL;
  }
  
  // ==========================================
  // 4. US-STYLE ÜBERSCHREIBUNGEN
  // ==========================================
  uint8_t front_r_left = pwm_left;
  uint8_t front_r_right = pwm_right;

  if (cfg.style_region == 1 || cfg.style_region == 2) {
    if (tfl_on && !car_is_parked) {
      if (!blink_left_active) front_r_left = BRIGHT_US_MARKER;
      if (!blink_right_active) front_r_right = BRIGHT_US_MARKER;
    }
    if (car_is_parked && tfl_on) {
      if (park_left_active && !blink_left_active) front_r_left = (BRIGHT_US_MARKER / 2);
      if (park_right_active && !blink_right_active) front_r_right = (BRIGHT_US_MARKER / 2);
    }
    target_front_g_left = 0;
    target_front_g_right = 0;
  }

  uint8_t rear_r_left = pwm_left; 
  uint8_t rear_r_right = pwm_right; 

  if (cfg.style_region == 2) {
    rear_r_left = 0;
    rear_r_right = 0;
    if (blink_left_active)  { target_rear_b_left = pwm_left;  cur_rear_b_left = target_rear_b_left; }
    if (blink_right_active) { target_rear_b_right = pwm_right; cur_rear_b_right = target_rear_b_right; }
  }

  // ==========================================
  // 5. FADING AUSFÜHREN (Statuswechsel & Post-Blinker)
  // ==========================================
  if (cfg.enable_soft_fade) {
    // Weiches Fading für das Hochfahren nach dem Blinken, bei Schalterwechseln und AUX
    cur_front_g_left = fadeTowards(cur_front_g_left, target_front_g_left, FADE_STEP);
    cur_front_g_right = fadeTowards(cur_front_g_right, target_front_g_right, FADE_STEP);
    cur_front_b = fadeTowards(cur_front_b, target_front_b, FADE_STEP);
    cur_aux_b = fadeTowards(cur_aux_b, target_aux_b, FADE_STEP); // NEU: AUX dimmt weich

    if (!car_is_braking && !(cfg.style_region == 2 && (blink_left_active || blink_right_active))) {
      cur_rear_b_left = fadeTowards(cur_rear_b_left, target_rear_b_left, FADE_STEP);
      cur_rear_b_right = fadeTowards(cur_rear_b_right, target_rear_b_right, FADE_STEP);
    }
  } 
  else {
    // Hartes Umschalten (Klassisch ohne Fading)
    cur_front_g_left = target_front_g_left;
    cur_front_g_right = target_front_g_right;
    cur_front_b = target_front_b;
    cur_aux_b = target_aux_b;

    if (!car_is_braking && !(cfg.style_region == 2 && (blink_left_active || blink_right_active))) {
      cur_rear_b_left = target_rear_b_left;
      cur_rear_b_right = target_rear_b_right;
    }
  }

  // Rückfahrscheinwerfer (bleibt hart, aus Sicherheits- und Reaktionsgründen)
  uint8_t rear_g = car_is_reversing ? BRIGHT_MAX : 0;

  // HAUPT-BELEUCHTUNG (stripMain)
  stripMain.setPixelColor(0, stripMain.Color(front_r_left, cur_front_g_left, cur_front_b)); 
  stripMain.setPixelColor(1, stripMain.Color(front_r_right, cur_front_g_right, cur_front_b)); 
  stripMain.setPixelColor(2, stripMain.Color(rear_r_right, rear_g, cur_rear_b_right));        
  stripMain.setPixelColor(3, stripMain.Color(rear_r_left, rear_g, cur_rear_b_left));          

  // ZUSATZ-BELEUCHTUNG (stripAux)
  stripAux.setPixelColor(0, stripAux.Color(0, 0, cur_aux_b));

  stripMain.show();
  stripAux.show();
}