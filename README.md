## 🚙 Traxxas TRX-4M Custom Light Controller
![AI Assisted](https://img.shields.io/badge/AI--Assisted-Gemini-blue?style=flat-for-the-badge&logo=googlegemini&logoColor=white) [![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Ein maßgeschneidertes, intelligentes Steuergerät (basierend auf dem **ESP32-C6**), um das **originale Traxxas TRX-4M Pro Scale Lichtkit (inkl. Lightbar)** mit 3rd-Party Fernsteuerungen (wie Flysky Noble NB4, Radiomaster, Futaba etc.) und Aftermarket-ESCs weiterverwenden zu können!

Wer seinen TRX-4M auf Brushless (z. B. Furitek) oder eine professionelle Funke umbaut, verliert normalerweise die Funktion des originalen Traxxas-Lichtsets, da dieses fest an die Traxxas ECM-2.5 Einheit gebunden ist. Dieses Projekt ersetzt die Traxxas-Elektronik als "Gehirn" für die Lichter und wertet das System mit smarten Features, Web-Konfiguration und Telemetrie-ähnlicher Auswertung extrem auf.

## ✨ Warum dieses Projekt?
Das originale TRX-4M Lichtkit nutzt adressierbare LEDs (ähnlich dem WS2812-Protokoll), die über eine serielle Datenleitung angesteuert werden. Wie genau wir die proprietären Signale entschlüsselt haben, kannst du in unserer detaillierten [Hardware & Protokoll Dokumentation (Reverse Engineering)](/docs/README.md) nachlesen.

Dieses Modul liest das Signal deines neuen Empfängers aus, berechnet die Fahrphysik und übersetzt sie in Echtzeit für das Traxxas-Lichtset. Du musst nichts abrüsten oder neu verkabeln – das Modul klinkt sich einfach als Übersetzer dazwischen!

## 🚀 Features
* **Multi-Protokoll für 3rd-Party Empfänger:** Unterstützt SBUS, iBUS und klassisches PPM. Ideal für Micro-Empfänger.
* **Smartphone Web-Interface:** Eigener WLAN-Access-Point zur einfachen Konfiguration direkt am Fahrzeug (Deutsch & Englisch). Kein PC auf dem Trail nötig!
* **Perfekte Anpassung an die Funke:**
  * **Volles Channel-Mapping:** Weise Funktionen wie Hauptlicht, Zusatzscheinwerfer und Blinker völlig frei beliebigen Empfängerkanälen zu.
  * **Auto-Trim-Funktion:** Ein Klick im Web-Interface und das Modul speichert den mechanischen Lenk-Offset (Nullpunkt) deiner Funke.
  * **Reverse-Schalter:** Die Laufrichtung jedes Kanals lässt sich direkt in der Software umkehren.
* **Erweiterte Blinker- & Licht-Logik:**
  * **Smart Auto-Cancel:** Blinker schaltet sich wie beim echten Auto ab, wenn du aus der Kurve lenkst.
  * **Blinker-Styles:** Wähle das optische Design (Classic, Soft-Off (Mazda), Soft-On (BMW), Retro Glühlampe).
  * **Regionale Licht-Profile (EU/US):** Verwandle die Optik deines Scalers mit einem Klick:
    * *EU-Style:* Klassisch getrennte Blinker (gelb) und Bremslichter (rot).
    * *US-Style Front:* Vordere Blinker glimmen dauerhaft gedimmt als orange Sidemarker (Standlicht) und pulsieren beim Blinken.
    * *US-Style Komplett:* Front-Sidemarker kombiniert mit typisch amerikanischem Heck (das rote Bremslicht blinkt im Takt, die gelben Blinker bleiben deaktiviert).
* **Adaptives Bremslicht (ESS):** Erkennt echte Vollbremsungen über den Gashebel und lässt das Bremslicht realistisch flackern. Danach schaltet sich automatisch der Warnblinker ein.
* **ESC-Kompatibilität & Fahrlogik:** Drei wählbare Profile für die perfekte Erkennung von Bremse und Rückfahrlicht:
  * *Standard:* Klassischer RC-Regler (Doppelklick für Rückwärts).
  * *Crawler (FOC):* Drag-Brake Erkennung und direkter Übergang in den Rückwärtsgang.
  * *Real-Car (Getriebe):* Manuelle Gangschaltung (Vorwärts / Rückwärts) über einen separaten AUX-Kanal für den ultimativen Scale-Realismus.
* **Standby & Parklicht:** Dimmt die Scheinwerfer nach definierter Inaktivität automatisch ab, um Strom zu sparen und Hitzestau zu vermeiden. Generelle Master-Helligkeit im Web-UI einstellbar.

## 🛠 Hardware & Verkabelung
Das Projekt läuft auf einem **Seeed Studio XIAO ESP32-C6** (oder ähnlichen, kompakten ESP32-Boards), da dieser klein genug ist, um problemlos in den TRX-4M zu passen.
* **Eingang:** Der ESP32 wird per 3-Pin-Kabel mit dem Empfänger verbunden (Strom, GND, Signal-Pin für SBUS/iBUS/PPM).
* **Ausgänge (2-Kanal System):** Die Ansteuerung der Traxxas-LEDs erfolgt über zwei getrennte Datenleitungen am ESP32.
  * **Kanal 1 (Main):** Übernimmt die komplette Basis-Fahrzeugbeleuchtung (Scheinwerfer, Rücklichter, Blinker).
  * **Kanal 2 (Aux):** Ist exklusiv für Zusatzscheinwerfer (z. B. Lightbars oder Roof-Lights) reserviert und lässt sich separat schalten.

*(Detaillierte Pin-Belegung und Schaltpläne für den Anschluss des Traxxas-Kabelbaums findest du im Wiki / in den Code-Kommentaren).*

## 💻 Installation (PlatformIO)
1. Repository klonen: 
    ``` bash
    git clone https://github.com/DEIN_NAME/trx4m-light-controller.git
    ```
2. Ordner in **VS Code** (mit PlatformIO) öffnen.
3. ESP32-C6 über USB-C anschließen.
4. Auf "Upload" klicken – PlatformIO kompiliert den Code und flasht den Chip automatisch.

## 📱 Konfiguration im Fahrzeug
1. TRX-4M einschalten. Der ESP32 öffnet ein WLAN-Netzwerk (z.B. ProScale-Config).
2. Smartphone verbinden und [http://192.168.4.1](http://192.168.4.1) im Browser öffnen.
3. Kanäle zuweisen, Blinker-Verhalten wählen und ggf. die Lenkung über den "Auto-Trim"-Button exakt nullen.
4. Speichern, Neustart abwarten – fertig für den Trail!

## 📄 Lizenz
Dieses Projekt ist unter der GNU General Public License v3.0 (GPLv3) lizenziert. Der Code ist Open Source. Details siehe LICENSE-Datei.
