/*
  ============================================================
   RELAY TO V6 ENGINE  -  ESP32 Throttle Sound Simulator Version 2
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
const int POT_PIN = 4;                               // Throttle pot (GPIO4 = ADC2_CH0, don't use WiFi)
const int RELAY_PINS[6] = {16, 17, 18, 19, 21, 22};  // Cylinders 1..6

// ---------------- Settings ----------------
const bool  RELAY_ACTIVE_LOW = false;  // true for active-LOW relay modules
const float FREQ_MIN = 30.0;           // Hz at R = 0   (idle)
const float FREQ_MAX = 90.0;           // Hz at R = 47k (full throttle)
const int   ADC_MAX  = 4095;
const int   SAMPLES  = 16;
const float SMOOTH   = 0.1;            // lower = smoother throttle response

const int MODE = 1;                    // 0 = all together, 1 = V6 firing order

// Firing order (0-based): cylinders 1-4-2-5-3-6
const int FIRING_ORDER[6] = {0, 3, 1, 4, 2, 5};

// ---------------- Variables ----------------
float filteredAdc = 0;
float frequency   = FREQ_MIN;
unsigned long halfPeriodUs = 16666;
unsigned long lastToggleUs = 0;
unsigned long lastPrintMs  = 0;
bool phaseOn   = false;
int  fireIndex = 0;

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
  analogSetPinAttenuation(POT_PIN, ADC_11db);

  for (int i = 0; i < 6; i++) pinMode(RELAY_PINS[i], OUTPUT);
  allOff();

  filteredAdc = readPotAveraged();

  Serial.println("=================================");
  Serial.println("   RELAY TO V6 ENGINE - STARTED  ");
  Serial.println("   YouTube  : YOUR_YOUTUBE_CHANNEL_LINK");
  Serial.println("   Instagram: YOUR_INSTAGRAM_LINK");
  Serial.println("=================================");
}

void loop() {
  // 1. Read throttle
  int raw = readPotAveraged();
  filteredAdc += SMOOTH * (raw - filteredAdc);

  // 2. Throttle -> firing frequency
  frequency = FREQ_MIN + (filteredAdc / ADC_MAX) * (FREQ_MAX - FREQ_MIN);
  frequency = constrain(frequency, FREQ_MIN, FREQ_MAX);
  halfPeriodUs = (unsigned long)(1000000.0 / (2.0 * frequency));

  // 3. Non-blocking engine tick
  unsigned long now = micros();
  if (now - lastToggleUs >= halfPeriodUs) {
    lastToggleUs += halfPeriodUs;
    if (now - lastToggleUs > halfPeriodUs) lastToggleUs = now;

    phaseOn = !phaseOn;

    if (MODE == 0) {
      for (int i = 0; i < 6; i++) writeRelay(i, phaseOn);
    } else {
      int cyl = FIRING_ORDER[fireIndex];
      writeRelay(cyl, phaseOn);          // ON = fire, OFF = release
      if (!phaseOn) fireIndex = (fireIndex + 1) % 6;  // next cylinder
    }
  }

  // 4. Dashboard
  if (millis() - lastPrintMs >= 500) {
    lastPrintMs = millis();
    float rpm = frequency / 3.0 * 60.0;  // V6: 3 firings per revolution
    Serial.printf("Throttle: %3d%%  |  %.1f Hz  |  ~%4.0f RPM\n",
                  (int)(filteredAdc * 100.0 / ADC_MAX), frequency, rpm);
  }
}
