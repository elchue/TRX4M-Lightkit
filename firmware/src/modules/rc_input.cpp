#include "rc_input.h"
#include "globals.h"

// ==========================================
// PPM INTERRUPT VARIABLEN & ISR
// ==========================================
volatile uint16_t ppm_raw[8] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};
volatile uint8_t ppm_current_channel = 0;
volatile unsigned long ppm_last_pulse_time = 0;
volatile bool ppm_new_data = false;

void readIBUS()
{
  static uint8_t buffer[32];
  static uint8_t index = 0;
  while (Serial1.available())
  {
    uint8_t c = Serial1.read();
    buffer[index++] = c;
    if (index == 1 && buffer[0] != 0x20)
      index = 0;
    else if (index == 2 && buffer[1] != 0x40)
      index = 0;
    else if (index == 32)
    {
      uint16_t chksum = 0xFFFF;
      for (int i = 0; i < 30; i++)
        chksum -= buffer[i];
      uint16_t rx_chksum = buffer[30] | (buffer[31] << 8);

      if (chksum == rx_chksum)
      {
        for (int i = 0; i < 14; i++)
        {
          rc_channels[i] = (buffer[2 + (i * 2)] | (buffer[3 + (i * 2)] << 8)) & 0x0FFF;
        }
        last_rc_time = millis();
      }
      index = 0;
    }
  }
}

void readSBUS()
{
  static uint8_t buffer[25];
  static uint8_t index = 0;
  while (Serial1.available())
  {
    uint8_t c = Serial1.read();
    if (index == 0 && c != 0x0F)
      continue;
    buffer[index++] = c;

    if (index == 25)
    {
      uint16_t ch[14];
      ch[0] = (buffer[1] | buffer[2] << 8) & 0x07FF;
      ch[1] = (buffer[2] >> 3 | buffer[3] << 5) & 0x07FF;
      ch[2] = (buffer[3] >> 6 | buffer[4] << 2 | buffer[5] << 10) & 0x07FF;
      ch[3] = (buffer[5] >> 1 | buffer[6] << 7) & 0x07FF;
      ch[4] = (buffer[6] >> 4 | buffer[7] << 4) & 0x07FF;
      ch[5] = (buffer[7] >> 7 | buffer[8] << 1 | buffer[9] << 9) & 0x07FF;
      ch[6] = (buffer[9] >> 2 | buffer[10] << 6) & 0x07FF;
      ch[7] = (buffer[10] >> 5 | buffer[11] << 3) & 0x07FF;
      ch[8] = (buffer[12] | buffer[13] << 8) & 0x07FF;
      ch[9] = (buffer[13] >> 3 | buffer[14] << 5) & 0x07FF;
      ch[10] = (buffer[14] >> 6 | buffer[15] << 2 | buffer[16] << 10) & 0x07FF;
      ch[11] = (buffer[16] >> 1 | buffer[17] << 7) & 0x07FF;
      ch[12] = (buffer[17] >> 4 | buffer[18] << 4) & 0x07FF;
      ch[13] = (buffer[18] >> 7 | buffer[19] << 1 | buffer[20] << 9) & 0x07FF;

      bool failsafe = (buffer[23] & (1 << 3));
      if (!failsafe)
      {
        for (int i = 0; i < 14; i++)
        {
          rc_channels[i] = map(ch[i], 172, 1811, 1000, 2000);
        }
        last_rc_time = millis();
      }
      index = 0;
    }
  }
}

// Die Interrupt-Routine (läuft unsichtbar im Hintergrund)
void IRAM_ATTR ppm_isr()
{
  unsigned long now = micros();
  unsigned long pulse_width = now - ppm_last_pulse_time;
  ppm_last_pulse_time = now;

  if (pulse_width > 2500)
  {
    ppm_current_channel = 0;
    ppm_new_data = true;
  }
  else if (ppm_current_channel < 8)
  {
    if (pulse_width > 800 && pulse_width < 2200)
    {
      ppm_raw[ppm_current_channel] = pulse_width;
    }
    ppm_current_channel = ppm_current_channel + 1;
  }
}

void readPPM()
{
  if (ppm_new_data)
  {
    noInterrupts();
    for (int i = 0; i < 8; i++)
    {
      rc_channels[i] = ppm_raw[i];
    }
    ppm_new_data = false;
    interrupts();

    last_rc_time = millis();
  }
}

void initRC()
{
  if (cfg.protocol == 0)
  {
    Serial1.begin(115200, SERIAL_8N1, PIN_RX_IN, -1);
    Serial.println("iBUS Parser lauscht...");
  }
  else if (cfg.protocol == 1)
  {
    Serial1.begin(100000, SERIAL_8E2, PIN_RX_IN, -1, true);
    Serial.println("SBUS Parser lauscht...");
  }
  else if (cfg.protocol == 2)
  {
    pinMode(PIN_RX_IN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_RX_IN), ppm_isr, RISING);
    Serial.println("PPM Parser lauscht...");
  }
}

void updateRC()
{
  if (cfg.protocol == 0)
    readIBUS();
  else if (cfg.protocol == 1)
    readSBUS();
  else if (cfg.protocol == 2)
    readPPM();

  if ((millis() - last_rc_time) < 500)
  {
    signalValid = true;
  }
  else
  {
    signalValid = false;
  }
}