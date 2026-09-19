#include "web_config.h"
#include "globals.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

AsyncWebServer server(80);
DNSServer dnsServer;
Preferences preferences;
IPAddress apIP(192, 168, 4, 1);

const String index_html = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ProScale Light Config</title>
  <style>
    :root {
      /* Neonrot Dark Theme */
      --bg-color: #0a0b0e;
      --card-bg: #12151c;
      --neon-red: #ff1e42;
      --neon-glow: rgba(255, 30, 66, 0.4);
      --neon-dark: rgba(255, 30, 66, 0.15);
      --purple-accent: #9d174d;
      --text-main: #f8fafc;
      --text-muted: #64748b;
      --border-color: #262c3a;
      
      /* Mapping für alte Inline-Styles */
      --primary: var(--neon-red);
    }

    * { box-sizing: border-box; }

    body { 
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; 
      background-color: var(--bg-color); 
      color: var(--text-main); 
      margin: 0; 
      padding: 20px 10px; 
      display: flex; 
      flex-direction: column; 
      align-items: center; 
    }

    .container { 
      display: flex; 
      flex-direction: column; 
      background-color: var(--card-bg); 
      padding: 24px; 
      border: 1px solid var(--border-color);
      border-radius: 16px; 
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.6); 
      max-width: 480px; 
      width: 95vw; 
    }

    header { text-align: center; margin-bottom: 24px; }
    
    h2 { 
      margin: 0; 
      font-size: 1.4rem; 
      font-weight: 400; 
      letter-spacing: 0.2em; 
      text-transform: uppercase; 
      color: var(--text-main); 
      text-shadow: 0 0 10px rgba(255, 255, 255, 0.2);
    }
    
    .version { 
      display: inline-block;
      margin-top: 8px;
      font-size: 0.65rem; 
      background: var(--neon-dark); 
      border: 1px solid var(--neon-red); 
      color: var(--neon-red); 
      padding: 4px 10px; 
      border-radius: 12px; 
      font-weight: 700; 
      letter-spacing: 0.1em;
      text-transform: uppercase;
      box-shadow: 0 0 8px var(--neon-glow);
    }

    form { display: flex; flex-direction: column; }
    
    label { font-weight: 600; font-size: 0.85rem; color: var(--text-main); letter-spacing: 0.05em; }
    
    select { 
      padding: 8px 12px; 
      background-color: var(--bg-color); 
      border: 1px solid var(--border-color); 
      color: var(--text-main); 
      border-radius: 8px; 
      font-size: 0.85rem; 
      outline: none;
      transition: all 0.3s ease;
      cursor: pointer;
    }
    select:focus {
      border-color: var(--neon-red);
      box-shadow: 0 0 12px var(--neon-glow);
    }
    
    .form-row { 
      display: flex; 
      justify-content: space-between; 
      align-items: center; 
      margin-top: 16px; 
      border-bottom: 1px solid var(--border-color); 
      padding-bottom: 12px; 
      transition: border-color 0.3s ease;
    }
    .form-row:hover { border-color: rgba(255, 30, 66, 0.3); }
    .form-row > label { flex: 1; padding-right: 5px; }
    
    .wide-select { flex: 0.6; min-width: 140px; }
    .control-group { display: flex; align-items: center; gap: 8px; flex: 0.6; min-width: 140px; justify-content: flex-end; }
    .control-group select { flex: 1; min-width: 80px; }
    
    /* Reverse Checkboxen */
    .rev-box { 
      display: flex; 
      align-items: center; 
      font-size: 0.7rem; 
      gap: 4px; 
      cursor: pointer; 
      color: var(--text-muted); 
      text-transform: uppercase;
      font-weight: 700;
      margin: 0; 
    }
    .rev-box input, .checkbox-group input { 
      margin: 0; 
      width: 18px; 
      height: 18px; 
      accent-color: var(--neon-red); 
      cursor: pointer;
    }

    /* Auto-Trim Button */
    .trim-btn { 
      margin: 0 8px; 
      padding: 6px 12px; 
      background: transparent; 
      color: var(--text-muted); 
      border: 1px solid var(--border-color); 
      border-radius: 20px; 
      font-size: 0.7rem; 
      font-weight: 600;
      text-transform: uppercase;
      cursor: pointer; 
      white-space: nowrap; 
      transition: all 0.2s ease;
    }
    .trim-btn:hover { border-color: var(--neon-red); color: var(--neon-red); }
    .trim-btn.success { 
      background: var(--neon-dark);
      border-color: var(--neon-red); 
      color: var(--neon-red); 
      box-shadow: 0 0 12px var(--neon-glow);
    }

    .checkbox-group { 
      display: flex; 
      align-items: center; 
      margin-top: 16px; 
      border-bottom: 1px solid var(--border-color); 
      padding-bottom: 12px; 
    }
    .checkbox-group input { margin-right: 12px; }
    
    /* Neonrot Speichern Button */
    button[type="submit"] { 
      margin-top: 30px; 
      padding: 14px; 
      border: none; 
      border-radius: 12px; 
      background: linear-gradient(135deg, #ff1e42, #b91c1c); 
      color: #ffffff; 
      font-size: 1rem; 
      font-weight: 700; 
      letter-spacing: 0.12em;
      text-transform: uppercase;
      cursor: pointer; 
      align-self: stretch; 
      box-shadow: 0 4px 20px var(--neon-glow);
      transition: all 0.2s ease;
    }
    button[type="submit"]:hover {
      box-shadow: 0 6px 25px rgba(255, 30, 66, 0.6);
    }
    button[type="submit"]:active { transform: scale(0.98); }
    
    .dynamic-field { display: none; transition: opacity 0.3s; }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h2>ProScale Config</h2>
      <div class="version">Firmware %VERSION%</div>
    </header>
    
    <div class="form-row" style="margin-bottom: 20px; border-bottom: 2px solid var(--neon-red);">
      <label data-i18n="lang">Sprache / Language</label>
      <select id="langSwitch" onchange="setLanguage(this.value)" style="flex: 0.4;">
        <option value="de">Deutsch</option>
        <option value="en">English</option>
      </select>
    </div>

    <form action="/save" method="GET">
      
      <input type="hidden" name="trim_steer" id="trim_steer_val" value="%TRIM_ST%">

      <div class="form-row">
        <label data-i18n="prot">Empfänger Protokoll</label>
        <select name="protocol" class="wide-select">
          <option value="0" %PROT_0%>iBUS</option>
          <option value="1" %PROT_1%>SBUS</option>
          <option value="2" %PROT_2%>PPM</option>
        </select>
      </div>
      
      <div class="form-row">
        <label data-i18n="ch_st">Lenkung</label>
        <button type="button" class="trim-btn" id="btn_trim" onclick="doAutoTrim()">Auto-Trim</button>
        <div class="control-group">
          <select name="ch_steering" class="channel-select" data-selected="%CH_ST%"></select>
          <label class="rev-box">
            <input type="checkbox" name="rev_steer" value="1" %REV_ST_CHK%>
            <span data-i18n="rev">Rev</span>
          </label>
        </div>
      </div>

      <div class="form-row">
        <label data-i18n="ch_th">Gas</label>
        <div class="control-group">
          <select name="ch_throttle" class="channel-select" data-selected="%CH_TH%"></select>
          <label class="rev-box">
            <input type="checkbox" name="rev_throttle" value="1" %REV_TH_CHK%>
            <span data-i18n="rev">Rev</span>
          </label>
        </div>
      </div>
      
      <div class="form-row">
        <label data-i18n="esc">ESC / Brems-Modus</label>
        <select name="mode_esc" id="mesc_select" onchange="checkModes()" class="wide-select">
          <option value="0" %MESC_0% data-i18n="esc0">Crawler (FOC)</option>
          <option value="1" %MESC_1% data-i18n="esc1">Standard (Doppelklick)</option>
          <option value="2" %MESC_2% data-i18n="esc2">Real-Car (Getriebe)</option>
        </select>
      </div>
      
      <div id="gear_container" class="dynamic-field">
        <div class="form-row">
          <label style="color: var(--neon-red);" data-i18n="ch_gear">Kanal: Getriebe</label>
          <div class="control-group">
            <select name="ch_gear" class="channel-select" data-selected="%CH_GEAR%"></select>
            <label class="rev-box">
              <input type="checkbox" name="rev_gear" value="1" %REV_GEAR_CHK%>
              <span data-i18n="rev">Rev</span>
            </label>
          </div>
        </div>
      </div>
      
      <div id="ess_container" class="checkbox-group">
        <input type="checkbox" name="ess" value="1" %ESS_CHK% id="ess_box">
        <label for="ess_box" data-i18n="ess">Adaptives Bremslicht (ESS)</label>
      </div>
      
      <div class="form-row">
        <label data-i18n="sreg">Licht-Profil (Region)</label>
        <select name="style_region" class="wide-select">
          <option value="0" %SREG_0% data-i18n="sreg0">EU Style (Separat)</option>
          <option value="1" %SREG_1% data-i18n="sreg1">US Style (Front-Marker)</option>
          <option value="2" %SREG_2% data-i18n="sreg2">US Komplett (Kombi-Heck)</option>
        </select>
      </div>

      <div class="checkbox-group">
        <input type="checkbox" name="enable_soft_fade" value="1" %FADE_CHK% id="fade_box">
        <label for="fade_box" data-i18n="fade">Soft-Fading für Hauptlicht</label>
      </div>

      <div class="form-row">
        <label data-i18n="blk">Blinker-Modus</label>
        <select name="mode_blinker" id="mblk_select" onchange="checkModes()" class="wide-select">
          <option value="0" %MBLK_0% data-i18n="blk0">Classic (Lenken)</option>
          <option value="1" %MBLK_1% data-i18n="blk1">Smart (Auto-Cancel)</option>
          <option value="2" %MBLK_2% data-i18n="blk2">Manual (Über AUX)</option>
        </select>
      </div>
      
      <div id="blinker_man_container" class="dynamic-field">
        <div class="form-row">
          <label style="color: var(--neon-red);" data-i18n="ch_blkm">Kanal: Blinker Man.</label>
          <div class="control-group">
            <select name="ch_blinker_man" class="channel-select" data-selected="%CH_BLK_MAN%"></select>
            <label class="rev-box">
              <input type="checkbox" name="rev_blinker_man" value="1" %REV_BLK_MAN_CHK%>
              <span data-i18n="rev">Rev</span>
            </label>
          </div>
        </div>
      </div>
      
      <div class="form-row">
        <label data-i18n="blkc">Blinker-Design</label>
        <select name="style_blinker" class="wide-select">
          <option value="0" %SBLK_0% data-i18n="blkc0">Classic (Hart An/Aus)</option>
          <option value="1" %SBLK_1% data-i18n="blkc1">Mazda (Soft-Off)</option>
          <option value="2" %SBLK_2% data-i18n="blkc2">BMW (Soft-On)</option>
          <option value="3" %SBLK_3% data-i18n="blkc3">Retro (Soft-Fade)</option>
        </select>
      </div>
      
      <div class="form-row">
        <label data-i18n="haz">Warnblinker</label>
        <div class="control-group">
          <select name="ch_hazard" class="channel-select" data-selected="%CH_HAZ%" data-allow-zero="true" data-zero-key="off"></select>
          <label class="rev-box">
            <input type="checkbox" name="rev_hazard" value="1" %REV_HAZ_CHK%>
            <span data-i18n="rev">Rev</span>
          </label>
        </div>
      </div>

      <div class="form-row">
        <label data-i18n="lgt">Hauptlicht</label>
        <div class="control-group">
          <select name="ch_light" class="channel-select" data-selected="%CH_LGT%" data-allow-zero="true" data-zero-key="on_always"></select>
          <label class="rev-box">
            <input type="checkbox" name="rev_light" value="1" %REV_LGT_CHK%>
            <span data-i18n="rev">Rev</span>
          </label>
        </div>
      </div>

      <div class="form-row">
        <label data-i18n="aux_l">Zusatz-Licht</label>
        <div class="control-group">
          <select name="ch_aux" class="channel-select" data-selected="%CH_AUX%" data-allow-zero="true" data-zero-key="on_always"></select>
          <label class="rev-box">
            <input type="checkbox" name="rev_aux" value="1" %REV_AUX_CHK%>
            <span data-i18n="rev">Rev</span>
          </label>
        </div>
      </div>
      
      <div class="form-row">
        <label data-i18n="park">Parklicht-Timeout</label>
        <select name="park_timeout" class="wide-select">
          <option value="0" %TPARK_0% data-i18n="p0">Aus (Immer an)</option>
          <option value="30" %TPARK_30% data-i18n="p30">30 Sekunden</option>
          <option value="60" %TPARK_60% data-i18n="p60">1 Minute</option>
          <option value="180" %TPARK_180% data-i18n="p180">3 Minuten</option>
        </select>
      </div>

      <div class="form-row">
        <label data-i18n="mbright">Gesamthelligkeit</label>
        <div style="display: flex; align-items: center; gap: 10px; flex: 0.6; min-width: 140px; justify-content: flex-end;">
          <input type="range" name="master_brightness" min="10" max="100" value="%MBRIGHT%" 
                 oninput="document.getElementById('mbright_val').innerText = this.value + '&#37;'" 
                 style="flex: 1; accent-color: var(--neon-red);">
          <span id="mbright_val" style="min-width: 35px; text-align: right; font-size: 0.9em; font-family: monospace; color: var(--text-main);">%MBRIGHT%&#37;</span>
        </div>
      </div>
      
      <button type="submit" data-i18n="save">Speichern & Neustart</button>
    </form>
  </div>

  <script>
    const dict = {
      de: {
        lang: "Sprache", prot: "Empfänger Protokoll", ch_st: "Lenkung", ch_th: "Gas",
        esc: "ESC / Brems-Modus", esc0: "Crawler (FOC)", esc1: "Standard (Doppelklick)", esc2: "Real-Car (Getriebe)",
        ch_gear: "Kanal: Getriebe", ess: "Adaptives Bremslicht (ESS)",
        sreg: "Licht-Profil (Region)", sreg0: "EU Style (Separat)", sreg1: "US Style (Front-Marker)", sreg2: "US Komplett (Kombi-Heck)",
        fade: "Soft-Fading für Hauptlicht",
        blk: "Blinker-Modus", blk0: "Classic (Lenken)", blk1: "Smart (Auto-Cancel)", blk2: "Manual (Über AUX)",
        ch_blkm: "Kanal: Blinker Man.", blkc: "Blinker-Design", blkc0: "Classic (Hart An/Aus)", blkc1: "Mazda (Soft-Off)", blkc2: "BMW (Soft-On)", blkc3: "Retro (Soft-Fade)",
        haz: "Warnblinker", lgt: "Hauptlicht", aux_l: "Zusatz-Licht", rev: "Rev",
        park: "Parklicht-Timeout", p0: "Aus (Immer an)", p30: "30 Sekunden", p60: "1 Minute", p180: "3 Minuten",
        save: "Speichern & Neustart", chan: "Kanal", off: "Aus", on_always: "Dauerhaft an",
        mbright: "Gesamthelligkeit"
      },
      en: {
        lang: "Language", prot: "Receiver Protocol", ch_st: "Steering", ch_th: "Throttle",
        esc: "ESC / Brake Mode", esc0: "Crawler (FOC)", esc1: "Standard (Double Click)", esc2: "Real-Car (Gearbox)",
        ch_gear: "Channel: Gearbox", ess: "Adaptive Brake Light (ESS)",
        sreg: "Light Profile (Region)", sreg0: "EU Style (Separate)", sreg1: "US Style (Front Markers)", sreg2: "US Complete (Combo Tail)",
        fade: "Soft-Fading for Headlights",
        blk: "Indicator Mode", blk0: "Classic (Steering)", blk1: "Smart (Auto-Cancel)", blk2: "Manual (Via AUX)",
        ch_blkm: "Channel: Ind. Manual", blkc: "Indicator Style", blkc0: "Classic (Hard On/Off)", blkc1: "Mazda (Soft-Off)", blkc2: "BMW (Soft-On)", blkc3: "Retro (Soft-Fade)",
        haz: "Hazards", lgt: "Main Light", aux_l: "Aux Light", rev: "Rev",
        park: "Park Light Timeout", p0: "Off (Always On)", p30: "30 Seconds", p60: "1 Minute", p180: "3 Minutes",
        save: "Save & Reboot", chan: "Channel", off: "Off", on_always: "Always On", 
        mbright: "Master Brightness"
      }
    };

    let currentLang = localStorage.getItem('prefLang');
    if (!currentLang) {
      currentLang = (navigator.language || navigator.userLanguage).startsWith('de') ? 'de' : 'en';
    }

    function setLanguage(lang) {
      if (!dict[lang]) lang = 'en';
      currentLang = lang;
      
      document.querySelectorAll('[data-i18n]').forEach(el => {
        el.innerText = dict[lang][el.getAttribute('data-i18n')];
      });
      
      document.getElementById('langSwitch').value = lang;
      localStorage.setItem('prefLang', lang);
      generateChannels(); 
    }

    function generateChannels() {
      document.querySelectorAll('.channel-select').forEach(function(sel) {
        let selected = sel.getAttribute('data-selected');
        let allowZero = sel.getAttribute('data-allow-zero');
        let zeroKey = sel.getAttribute('data-zero-key'); 
        
        let html = '';
        if (allowZero === 'true') {
          html += '<option value="0">' + dict[currentLang][zeroKey] + '</option>';
        }
        for (let i = 1; i <= 18; i++) {
          html += '<option value="' + i + '">' + dict[currentLang].chan + ' ' + i + '</option>';
        }
        sel.innerHTML = html;
        if(selected && selected !== "") sel.value = selected;
      });
    }

    function checkModes() {
      let escMode = document.getElementById("mesc_select").value;
      let essContainer = document.getElementById("ess_container");
      let gearContainer = document.getElementById("gear_container");
      
      if(escMode == "0") {
        essContainer.style.opacity = "0.3";
        document.getElementById("ess_box").disabled = true;
        document.getElementById("ess_box").checked = false;
      } else {
        essContainer.style.opacity = "1";
        document.getElementById("ess_box").disabled = false;
      }
      gearContainer.style.display = (escMode == "2") ? "block" : "none";
      
      let blkMode = document.getElementById("mblk_select").value;
      let blkManContainer = document.getElementById("blinker_man_container");
      blkManContainer.style.display = (blkMode == "2") ? "block" : "none";
    }

    function doAutoTrim() {
      let btn = document.getElementById('btn_trim');
      let chSteerSelect = document.querySelector('select[name="ch_steering"]');
      let channel = chSteerSelect.value;
      
      if(channel == 0) {
        alert(currentLang === 'de' ? "Bitte zuerst einen Kanal für die Lenkung auswählen!" : "Please select a steering channel first!");
        return;
      }

      btn.innerText = "...";
      
      fetch('/api/trim?ch=' + channel)
        .then(response => response.json())
        .then(data => {
          if(data.success) {
            let offset = 1500 - data.raw_value; 
            document.getElementById('trim_steer_val').value = offset;
            
            btn.classList.add("success");
            btn.innerText = (offset > 0 ? "+" : "") + offset;
            setTimeout(() => { btn.classList.remove("success"); btn.innerText = "Auto-Trim"; }, 3000);
          } else {
            alert("Signalfehler!");
            btn.innerText = "Auto-Trim";
          }
        })
        .catch(error => {
          alert("Fehler beim Abrufen der Trimmung.");
          btn.innerText = "Auto-Trim";
        });
    }

    window.onload = function() {
      setLanguage(currentLang);
      checkModes();
      
      let currentTrim = parseInt(document.getElementById('trim_steer_val').value);
      if(currentTrim !== 0 && !isNaN(currentTrim)) {
        let btn = document.getElementById('btn_trim');
        btn.innerText = "Trim " + (currentTrim > 0 ? "+" : "") + currentTrim;
      }
    };
  </script>
</body>
</html>
)rawliteral";

String processor(const String &var)
{
  if (var == "VERSION") return FIRMWARE_VERSION;

  // Protokoll
  if (var == "PROT_0" && cfg.protocol == 0) return "selected";
  if (var == "PROT_1" && cfg.protocol == 1) return "selected";
  if (var == "PROT_2" && cfg.protocol == 2) return "selected";

  // Channels
  if (var == "CH_ST") return String(cfg.ch_steer);
  if (var == "CH_TH") return String(cfg.ch_throttle);
  if (var == "CH_LGT") return String(cfg.ch_light);
  if (var == "CH_AUX") return String(cfg.ch_aux);
  if (var == "CH_BLK_MAN") return String(cfg.ch_blinker_man);
  if (var == "CH_HAZ") return String(cfg.ch_hazard);
  if (var == "CH_GEAR") return String(cfg.ch_gear);

  // Reverse Checkboxen
  if (var == "REV_ST_CHK" && cfg.rev_steer) return "checked";
  if (var == "REV_TH_CHK" && cfg.rev_throttle) return "checked";
  if (var == "REV_LGT_CHK" && cfg.rev_light) return "checked";
  if (var == "REV_AUX_CHK" && cfg.rev_aux) return "checked";
  if (var == "REV_BLK_MAN_CHK" && cfg.rev_blinker_man) return "checked";
  if (var == "REV_HAZ_CHK" && cfg.rev_hazard) return "checked";
  if (var == "REV_GEAR_CHK" && cfg.rev_gear) return "checked";

  // Trimmung Lenkung
  if (var == "TRIM_ST") return String(cfg.trim_steer);

  // ESC / Brems-Modus
  if (var == "MESC_0" && cfg.mode_esc == 0) return "selected";
  if (var == "MESC_1" && cfg.mode_esc == 1) return "selected";
  if (var == "MESC_2" && cfg.mode_esc == 2) return "selected";
  if (var == "ESS_CHK" && cfg.ess_active) return "checked";
  
  // Licht-Profil (Region)
  if (var == "SREG_0" && cfg.style_region == 0) return "selected";
  if (var == "SREG_1" && cfg.style_region == 1) return "selected";
  if (var == "SREG_2" && cfg.style_region == 2) return "selected";
  if (var == "FADE_CHK" && cfg.enable_soft_fade) return "checked";
  //if (var == "DIM_CHK" && cfg.enable_tfl_dim_blink) return "checked";

  // Blinker-Modus
  if (var == "MBLK_0" && cfg.mode_blinker == 0) return "selected";
  if (var == "MBLK_1" && cfg.mode_blinker == 1) return "selected";
  if (var == "MBLK_2" && cfg.mode_blinker == 2) return "selected";

  // Blinker-Charakteristik
  if (var == "SBLK_0" && cfg.style_blinker == 0) return "selected";
  if (var == "SBLK_1" && cfg.style_blinker == 1) return "selected";
  if (var == "SBLK_2" && cfg.style_blinker == 2) return "selected";
  if (var == "SBLK_3" && cfg.style_blinker == 3) return "selected";

  // Parklicht-Timeout
  if (var == "TPARK_0" && cfg.timeout_park == 0) return "selected";
  if (var == "TPARK_30" && cfg.timeout_park == 30) return "selected";
  if (var == "TPARK_60" && cfg.timeout_park == 60) return "selected";
  if (var == "TPARK_180" && cfg.timeout_park == 180) return "selected";

  // Master Helligkeit
  if (var == "MBRIGHT") return String(cfg.master_brightness);

  return String();
}

void loadConfig()
{
  preferences.begin("proscale", false);
  cfg.protocol = preferences.getInt("protocol", 0);
  cfg.ch_steer = preferences.getInt("ch_steering", 1);
  cfg.ch_throttle = preferences.getInt("ch_throttle", 2);
  cfg.ch_light = preferences.getInt("ch_light", 0);
  cfg.ch_aux = preferences.getInt("ch_aux", 0);
  cfg.ch_blinker_man = preferences.getInt("ch_blinker_man", 0);
  cfg.ch_hazard = preferences.getInt("ch_hazard", 0);
  cfg.ch_gear = preferences.getInt("ch_gear", 0);
  
  // Reverse Checkboxen
  cfg.rev_steer = preferences.getBool("rev_steer", false);
  cfg.rev_throttle = preferences.getBool("rev_throttle", false);
  cfg.rev_light = preferences.getBool("rev_light", false);
  cfg.rev_aux = preferences.getBool("rev_aux", false);
  cfg.rev_blinker_man = preferences.getBool("rev_blinker_man", false);
  cfg.rev_hazard = preferences.getBool("rev_hazard", false);
  cfg.rev_gear = preferences.getBool("rev_gear", false);
  
  cfg.trim_steer = preferences.getInt("trim_steer", 0);
  cfg.mode_esc = preferences.getInt("mode_esc", 1);
  cfg.ess_active = preferences.getBool("ess", false);
  
  cfg.style_region = preferences.getInt("style_region", 0);
  cfg.enable_soft_fade = preferences.getBool("soft_fade", true);
  //cfg.enable_tfl_dim_blink = preferences.getBool("tfl_dim", true);

  cfg.master_brightness = preferences.getInt("master_brightness", 100);
  
  cfg.mode_blinker = preferences.getInt("mode_blinker", 0);
  cfg.style_blinker = preferences.getInt("style_blinker", 0);
  cfg.timeout_park = preferences.getInt("park_timeout", 60);
  preferences.end();
}

void saveConfig(AsyncWebServerRequest *request)
{
  preferences.begin("proscale", false);
  if (request->hasParam("protocol")) preferences.putInt("protocol", request->getParam("protocol")->value().toInt());
  if (request->hasParam("ch_steering")) preferences.putInt("ch_steering", request->getParam("ch_steering")->value().toInt());
  if (request->hasParam("ch_throttle")) preferences.putInt("ch_throttle", request->getParam("ch_throttle")->value().toInt());
  if (request->hasParam("ch_light")) preferences.putInt("ch_light", request->getParam("ch_light")->value().toInt());
  if (request->hasParam("ch_aux")) preferences.putInt("ch_aux", request->getParam("ch_aux")->value().toInt());
  if (request->hasParam("ch_blinker_man")) preferences.putInt("ch_blinker_man", request->getParam("ch_blinker_man")->value().toInt());
  if (request->hasParam("ch_hazard")) preferences.putInt("ch_hazard", request->getParam("ch_hazard")->value().toInt());
  if (request->hasParam("ch_gear")) preferences.putInt("ch_gear", request->getParam("ch_gear")->value().toInt());
  
  // Reverse Checkboxen
  preferences.putBool("rev_steer", request->hasParam("rev_steer"));
  preferences.putBool("rev_throttle", request->hasParam("rev_throttle"));
  preferences.putBool("rev_light", request->hasParam("rev_light"));
  preferences.putBool("rev_aux", request->hasParam("rev_aux"));
  preferences.putBool("rev_blinker_man", request->hasParam("rev_blinker_man"));
  preferences.putBool("rev_hazard", request->hasParam("rev_hazard"));
  preferences.putBool("rev_gear", request->hasParam("rev_gear"));
  
  if (request->hasParam("trim_steer")) preferences.putInt("trim_steer", request->getParam("trim_steer")->value().toInt());
  if (request->hasParam("mode_esc")) preferences.putInt("mode_esc", request->getParam("mode_esc")->value().toInt());
  if (request->hasParam("style_region")) preferences.putInt("style_region", request->getParam("style_region")->value().toInt());
  if (request->hasParam("master_brightness")) preferences.putInt("master_brightness", request->getParam("master_brightness")->value().toInt());
  preferences.putBool("soft_fade", request->hasParam("enable_soft_fade"));
  //preferences.putBool("tfl_dim", request->hasParam("enable_tfl_dim_blink"));

  if (request->hasParam("mode_blinker")) preferences.putInt("mode_blinker", request->getParam("mode_blinker")->value().toInt());
  if (request->hasParam("style_blinker")) preferences.putInt("style_blinker", request->getParam("style_blinker")->value().toInt());
  if (request->hasParam("park_timeout")) preferences.putInt("park_timeout", request->getParam("park_timeout")->value().toInt());
  
  preferences.putBool("ess", request->hasParam("ess"));
  preferences.end();
}

void initWebConfig()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(WIFI_SSID, WIFI_PASS);

  dnsServer.start(53, "*", apIP);

  WiFi.onEvent([](arduino_event_id_t event, arduino_event_info_t info) { 
    clientDisconnected = true; 
  }, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { 
    request->send(200, "text/html", index_html, processor); 
  });

  server.on("/api/trim", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ch")) {
      int ch = request->getParam("ch")->value().toInt();
      if (ch > 0 && ch <= 18) {
        int raw_val = rc_channels[ch - 1];
        String json = "{\"success\":true,\"raw_value\":" + String(raw_val) + "}";
        request->send(200, "application/json", json);
        return;
      }
    }
    request->send(400, "application/json", "{\"success\":false}"); 
  }); 

  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request) {
    saveConfig(request);
    request->send(200, "text/html", "<h2 style='color:#f1faee; font-family:sans-serif;'>Gespeichert! Neustart...</h2>");
    delay(1000);
    ESP.restart(); 
  });

  server.onNotFound([](AsyncWebServerRequest *request) { 
    request->redirect("http://192.168.4.1/"); 
  });

  server.begin();
  Serial.println("WLAN Setup gestartet.");
}

void handleWebConfig()
{
  dnsServer.processNextRequest();
  if (clientDisconnected)
  {
    WiFi.softAPdisconnect(true);
    delay(500);
    ESP.restart();
  }
}