# TRX-4M Pro Scale Lichtsystem - Reverse Engineering

Dieses Repository enthält die Hardware-Zuordnung und Logik, um das proprietäre Traxxas TRX-4M Pro Scale Lichtsystem (Light Kit) sowie das originale Zubehör (Anhänger-Beleuchtung, Lightbars) mit eigenen Mikrocontrollern (z. B. ESP32, Arduino, SAMD21) und Standard-RC-Empfängern (wie Flysky iBUS) zu steuern.

## Hardware-Aufbau und das Y-Kabel-Prinzip

Das Traxxas Lichtkabel basiert technisch auf dem **WS2811/WS2812 (NeoPixel)** Protokoll. Anstatt jedoch echte RGB-LEDs zu verbauen, die ihre Farbe wechseln können, nutzt Traxxas unsichtbare Controller-Chips auf den Platinen, die das Signal empfangen. An die drei Farbausgänge (Rot, Grün, Blau) eines einzelnen Chips hat Traxxas hardwareseitig **drei verschiedene, einfarbige LEDs** gelötet. Ein Befehl für "100% Rot" schaltet also keine rote Farbe ein, sondern aktiviert einen spezifischen Funktionsausgang (z. B. den Blinker).

### Besonderheit: Die Verkabelung am ECM-2.5
Am originalen Traxxas **ECM-2.5** (Empfänger/ESC-Einheit) wird das Datensignal teilweise über physische Y-Kabel aufgeteilt:
1. **Haupt-Lichtkit:** Steuert die vier Ecken des Fahrzeugs.
2. **Zusatzscheinwerfer:** (Lightbar + Kühlergrill) Hängen an einem Y-Kabel und lesen denselben Datenstrom wie das vordere linke Abblendlicht mit.
3. **Anhänger:** Hängt ebenfalls an einem Y-Kabel. Die Chips im Anhänger sind so konfiguriert, dass sie synchron auf die Daten der hinteren Fahrzeugbeleuchtung (Chip 2 und 3) reagieren.

## Die LED-Matrix (Hardware Mapping)

Das gesamte System lässt sich im Code wie ein Strang aus **4 NeoPixel-Chips** (Index 0 bis 3) ansteuern. Die Helligkeit der Lampen kann über PWM-Werte (0 = Aus, 255 = Hell) stufenlos gedimmt werden.

| Chip Index | Farb-Kanal im Code | Funktion am Fahrzeug | Funktion am Anhänger |
| :--- | :--- | :--- | :--- |
| **Chip 0** | Rot | Blinker vorne Links | - |
| **Chip 0** | Grün | Tagfahrlicht Links | - |
| **Chip 0** | Blau | Abblendlicht Links + Zusatzscheinwerfer* | - |
| **Chip 1** | Rot | Blinker vorne Rechts | - |
| **Chip 1** | Grün | Tagfahrlicht Rechts | - |
| **Chip 1** | Blau | Abblendlicht Rechts | - |
| **Chip 2** | Rot | Blinker hinten Rechts | Blinker Rechts (rot) |
| **Chip 2** | Grün | Rückfahrlicht Rechts (weiß) | Rückfahrlicht Rechts |
| **Chip 2** | Blau | Rücklicht / Bremslicht Rechts | Bremslicht Rechts |
| **Chip 3** | Rot | Blinker hinten Links | Blinker Links (rot) |
| **Chip 3** | Grün | Rückfahrlicht Links (weiß) | Rückfahrlicht Links |
| **Chip 3** | Blau | Rücklicht / Bremslicht Links | Bremslicht Links |

*\*Hinweis zu den Zusatzscheinwerfern: Da diese über ein Y-Kabel fest mit dem blauen Kanal von Chip 0 gekoppelt sind, leuchten sie im Werkszustand immer synchron mit dem Abblendlicht auf.*

## Modding-Potenzial: Unabhängige Steuerung

Da das Traxxas ECM-2.5 die Stränge per Y-Kabel zusammenschließt, sind Anhänger und Lightbar im Originalzustand an die Hauptbeleuchtung gekoppelt. 

Wenn man das System mit einem eigenen Mikrocontroller steuert, können die Y-Kabel entfernt und die Komponenten an **separate GPIO-Pins** des Mikrocontrollers (z. B. 3 getrennte NeoPixel-Instanzen in der Software) angeschlossen werden. 
Dies ermöglicht völlig neue Funktionen, wie zum Beispiel:
* Zusatzscheinwerfer (Dach/Grill) unabhängig vom Abblendlicht per Fernsteuerung schalten.
* Anhängerbeleuchtung abweichend vom Zugfahrzeug steuern (z. B. Warnblinker nur am Anhänger).

## Programmierung (Beispiel)

Zur Ansteuerung kann jede handelsübliche Bibliothek für adressierbare LEDs genutzt werden (z. B. `Adafruit_NeoPixel` oder `FastLED`).

Um das Bremslicht hinten links am Fahrzeug (und zeitgleich am Anhänger, falls per Y-Kabel verbunden) auf 100% Helligkeit einzuschalten, wird der "Blaue" Kanal von Chip 3 angesteuert:
`strip.setPixelColor(3, 0, 0, 255)`
