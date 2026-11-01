/*
  ============================================================
   RELAY TO V6 ENGINE  -  ESP32 Throttle Sound Simulator
  ============================================================
   Turn a bank of clicking relays into a screaming V6!

   - 47k potentiometer = THROTTLE PEDAL (GPIO4)
   - 6 relays = 6 CYLINDERS
   - 30 Hz (pot = 0 ohm)  -> idle   (~600 RPM)
   - 90 Hz (pot = 47k)    -> redline (~1800 RPM sound pitch)
     (A 4-stroke V6 fires 3 times per revolution:
      firing Hz = RPM / 60 * 3)

   Modes:
     MODE 0 = all relays together (one big buzz)
     MODE 1 = V6 MODE: relays fire one by one in firing order

   YouTube  : https://youtube.com/@pandustechlab
   Instagram: https://www.instagram.com/pandutechlab

   WARNING: Run the relays with NO load / no mains voltage
            (sound effect only). Mechanical relays wear out fast
            at 30-90 Hz. Use cheap relays you can replace.
  ============================================================
*/

#include <Arduino.h>

// ---------------- Pin configuration ----------------
const int POT_PIN = 4;                              // ADC1 input-only pin
const int RELAY_PINS[6] = {16, 17, 18, 19, 21, 22};  // CH1..CH6

// ---------------- Settings ----------------
const bool  RELAY_ACTIVE_LOW = false;  // set true for active-LOW modules
const float FREQ_MIN = 30.0;           // Hz at R = 0
const float FREQ_MAX = 90.0;           // Hz at R = 47k
const int   ADC_MAX  = 4095;           // 12-bit ADC
const int   SAMPLES  = 16;             // averaging samples
const float SMOOTH   = 0.1;            // 0..1, lower = smoother

// Mode: 0 = all channels together, 1 = channels fire one after another
const int MODE = 0;

// ---------------- Variables ----------------
float filteredAdc = 0;
float frequency   = FREQ_MIN;
unsigned long halfPeriodUs = 16666;
unsigned long lastToggleUs = 0;
unsigned long lastPrintMs  = 0;
bool state = false;
int  seqIndex = 0;

inline void writeRelay(int ch, bool on) {
  digitalWrite(RELAY_PINS[ch], (on ^ RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

void allOff() {
  for (int i = 0; i < 6; i++) writeRelay(i, false);
}

int readPotAveraged() {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) sum += analogRead(POT_PIN);
  return sum / SAMPLES;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(POT_PIN, ADC_11db);  // full 0-3.3V range

  for (int i = 0; i < 6; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
  }
  allOff();

  filteredAdc = readPotAveraged();
  Serial.println("ESP32 Pot -> Relay Frequency Controller started");
}

void loop() {
  // 1. Read and smooth potentiometer
  int raw = readPotAveraged();
  filteredAdc += SMOOTH * (raw - filteredAdc);

  // 2. Map ADC value to frequency (30-80 Hz)
  frequency = FREQ_MIN + (filteredAdc / ADC_MAX) * (FREQ_MAX - FREQ_MIN);
  frequency = constrain(frequency, FREQ_MIN, FREQ_MAX);

  // 3. Half period in microseconds (50% duty)
  halfPeriodUs = (unsigned long)(1000000.0 / (2.0 * frequency));

  // 4. Non-blocking toggle
  unsigned long now = micros();
  if (now - lastToggleUs >= halfPeriodUs) {
    lastToggleUs += halfPeriodUs;
    if (now - lastToggleUs > halfPeriodUs) lastToggleUs = now;  // resync if late

    if (MODE == 0) {
      state = !state;
      for (int i = 0; i < 6; i++) writeRelay(i, state);
    } else {
      allOff();
      writeRelay(seqIndex, true);
      seqIndex = (seqIndex + 1) % 6;
    }
  }

  // 5. Serial monitor output
  if (millis() - lastPrintMs >= 500) {
    lastPrintMs = millis();
    Serial.printf("ADC: %4d  |  Frequency: %.1f Hz  |  Period: %.2f ms\n",
                  (int)filteredAdc, frequency, 1000.0 / frequency);
  }
}
