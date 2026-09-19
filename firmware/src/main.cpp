#include <Arduino.h>
#include "modules/globals.h"
#include "modules/rc_input.h"
#include "modules/web_config.h"
#include "modules/led_output.h"
#include "modules/drive_logic.h"

// ==========================================
// GLOBALE VARIABLEN DEKLARATION (Hier erwachen sie zum Leben)
// ==========================================
Config cfg;
uint16_t rc_channels[14] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};
unsigned long last_rc_time = 0;
bool signalValid = false;

bool configMode = false;
volatile bool clientDisconnected = false;
unsigned long letzteBewegung = 0;

Adafruit_NeoPixel stripMain(NUM_LEDS_MAIN, PIN_LED_MAIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel stripAux(NUM_LEDS_AUX, PIN_LED_AUX, NEO_RGB + NEO_KHZ800);

// ==========================================
// STATUS / HEARTBEAT EFFEKT (WLAN & Failsafe)
// ==========================================
void showStatusEffect(uint8_t peak_brightness) {
  float breath = (sin(millis() / 250.0) + 1.0) / 2.0; 
  uint8_t val = (uint8_t)(breath * peak_brightness);

  stripMain.setPixelColor(0, stripMain.Color(val, 0, 0));
  stripMain.setPixelColor(1, stripMain.Color(val, 0, 0));
  stripMain.setPixelColor(2, stripMain.Color(val, 0, 0));
  stripMain.setPixelColor(3, stripMain.Color(val, 0, 0));

  stripAux.setPixelColor(0, stripAux.Color(0, 0, 0));

  stripMain.show();
  stripAux.show();
}

// ==========================================
// SETUP
// ==========================================
void setup()
{
  Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);

  // --- RF ANTENNEN LOGIK (XIAO ESP32-C6) ---
  pinMode(PIN_RF_EN, OUTPUT);
  digitalWrite(PIN_RF_EN, LOW);  // RF Enable
  pinMode(PIN_RF_SEL, OUTPUT);
  digitalWrite(PIN_RF_SEL, LOW); // Interne Antenne

  // --- USER LED LOGIK ---
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW); // LED aus im Normalbetrieb

  // --- ANTI-FLICKER (Software Pull-Down vor LED Init) ---
  pinMode(0, OUTPUT); digitalWrite(0, LOW);
  pinMode(1, OUTPUT); digitalWrite(1, LOW);

  initLEDs();
  loadConfig();
  delay(100);

  // Fallback: Taster beim direkten Start gedrückt -> WLAN Setup
  if (digitalRead(PIN_BTN) == LOW)
  {
    configMode = true;
    digitalWrite(PIN_LED, LOW); // Status-LED an
    initWebConfig();
    return; // Das visuelle Feedback passiert ab jetzt automatisch in der loop()
  }

  // Normaler Fahrbetrieb
  Serial.println("Starte Fahrbetrieb...");
  initRC();
}

// ==========================================
// MAIN LOOP
// ==========================================
void loop()
{
  // 1. WEB CONFIG MODUS BLOCKIERT DEN REST
  if (configMode)
  {
    handleWebConfig();
    showStatusEffect(BRIGHT_DIMMED); // WLAN = Sanftes Atmen (gedimmt)
    return;
  }

  // 2. TASTER AUSWERTEN (3 Sekunden halten für Setup)
  static unsigned long btnPressStart = 0;
  static bool btnIsPressed = false;

  if (digitalRead(PIN_BTN) == LOW) 
  {
    if (!btnIsPressed) {
      btnIsPressed = true;
      btnPressStart = millis(); // Zeitstempel beim ersten Drücken merken
    } 
    else if (millis() - btnPressStart > 3000) // 3000 ms (3 Sekunden) gehalten
    { 
      Serial.println("Wechsle in WLAN Setup-Modus...");
      configMode = true;
      digitalWrite(PIN_LED, LOW); // Status-LED auf dem Board an
      initWebConfig();
      return; // Loop hier abbrechen, im nächsten Durchlauf greift Block 1
    }
  } 
  else 
  {
    btnIsPressed = false; // Taster losgelassen, Timer Reset
  }

  // 3. RC Daten aktualisieren
  updateRC();

  // 4. Kanäle auslesen
  int valSteering = 1500;
  int valThrottle = 1500;
  int valLight = 1500;
  int valAux = 1500;
  int valBlinkerMan = 1500;
  int valHazard = 1500;
  int valGear = 1500;

  // 5. Kanäle auslesen und manipulieren (Trim & Reverse)
  if (signalValid)
  {
    // --- LENKUNG (mit Trim & Reverse) ---
    if (cfg.ch_steer > 0)
    {
      valSteering = rc_channels[cfg.ch_steer - 1] + cfg.trim_steer;
      if (cfg.rev_steer) valSteering = 3000 - valSteering;
    }

    // --- GAS / BREMSE ---
    if (cfg.ch_throttle > 0)
    {
      valThrottle = rc_channels[cfg.ch_throttle - 1];
      if (cfg.rev_throttle) valThrottle = 3000 - valThrottle;
    }

    // --- HAUPTLICHT ---
    if (cfg.ch_light > 0)
    {
      valLight = rc_channels[cfg.ch_light - 1];
      if (cfg.rev_light) valLight = 3000 - valLight;
    }

    // --- ZUSATZLICHT (AUX) ---
    if (cfg.ch_aux > 0)
    {
      valAux = rc_channels[cfg.ch_aux - 1];
      if (cfg.rev_aux) valAux = 3000 - valAux;
    }

    // --- MANUELLER BLINKER ---
    if (cfg.ch_blinker_man > 0)
    {
      valBlinkerMan = rc_channels[cfg.ch_blinker_man - 1];
      if (cfg.rev_blinker_man) valBlinkerMan = 3000 - valBlinkerMan;
    }

    // --- WARNBLINKER ---
    if (cfg.ch_hazard > 0)
    {
      valHazard = rc_channels[cfg.ch_hazard - 1];
      if (cfg.rev_hazard) valHazard = 3000 - valHazard;
    }

    // --- GETRIEBE ---
    if (cfg.ch_gear > 0)
    {
      valGear = rc_channels[cfg.ch_gear - 1];
      if (cfg.rev_gear) valGear = 3000 - valGear;
    }

    // 6. Fahrlogik berechnen
    updateDriveLogic(valThrottle, valSteering, valGear);

    // 7. LEDs ansteuern!
    updateLEDs(valThrottle, valSteering, valLight, valAux, valBlinkerMan, valHazard);
  }
  else 
  {
    // FAILSAFE ANZEIGE: Wenn das RC-Signal weg ist, warnt das Fahrzeug
    showStatusEffect(BRIGHT_MAX); // Failsafe = Helles, deutliches Atmen
  }

  // 8. Debug Ausgabe
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 200)
  {
    lastDebug = millis();
    if (signalValid)
    {
      Serial.printf("CH1:%4d CH2:%4d CH3:%4d CH4:%4d CH5:%4d CH6:%4d CH7:%4d CH8:%4d | FWD:%d REV:%d BRK:%d PRK:%d\n",
                    rc_channels[0], rc_channels[1], rc_channels[2], rc_channels[3],
                    rc_channels[4], rc_channels[5], rc_channels[6], rc_channels[7],
                    car_is_driving, car_is_reversing, car_is_braking, car_is_parked);
    }
    else
    {
      Serial.println("Failsafe aktiv...");
    }
  }
}