# 🏎️💥 RELAY TO V6 ENGINE 💥🏎️

### *Who needs a car when you've got 6 relays and an ESP32?*

> Turn a bunch of clicking relays into a **screaming V6 engine** with a single potentiometer as your **throttle pedal**. Twist it, rev it, make your neighbours question everything.

[![YouTube](https://img.shields.io/badge/YouTube-Subscribe-red?logo=youtube&logoColor=white)](YOUR_YOUTUBE_CHANNEL_LINK)
[![Instagram](https://img.shields.io/badge/Instagram-Follow-E4405F?logo=instagram&logoColor=white)](YOUR_INSTAGRAM_LINK)

---

## 🔥 What is this?

This is an **ESP32-powered V6 engine sound simulator**. Six relays act as six cylinders and click in firing order. A **47 kΩ potentiometer** is your throttle:

| Throttle (Pot) | Relay firing rate | Engine feel |
|---|---|---|
| 0 Ω (foot off) | **30 Hz** | 🐢 Idle, ~600 RPM |
| 23.5 kΩ (half) | **~60 Hz** | 🚗 Cruising, ~1200 RPM |
| 47 kΩ (floored) | **90 Hz** | 🚀 Screaming, ~1800 RPM pitch |

Why the RPM numbers? A 4-stroke V6 fires **3 times per revolution**, so `firing Hz = RPM / 60 × 3`.

Formula in code: `f = 30 + (ADC / 4095) × 60`

---

## ⚠️ Read This Before You Burn Your Relays

- **Sound-effect only!** Run the relays with **no load and no mains voltage**.
- Mechanical relays are rated for ~10 switches per second. At 30–90 Hz they will buzz, chatter and **wear out quickly**. That's what makes the sound, but use cheap, replaceable relays.
- Want the same control without the clicking (e.g. real loads)? Swap in a **6-channel SSR/MOSFET board**. Wiring and code stay the same.
- Put the relays in a box or on a soft surface so the vibration doesn't walk them off the desk.

---

## 🧰 Components

| Qty | Component |
|---|---|
| 1 | ESP32 DevKit |
| 1 | 47 kΩ linear potentiometer (B47K), the throttle pedal |
| 1 | 6-channel relay module (5 V) |
| 1 | 0.1 µF ceramic capacitor (optional, smooths the throttle) |
| – | Jumper wires, breadboard, USB cable |

---

## 🔌 Pin Connections

### Throttle (Potentiometer) → ESP32

| Pot pin | ESP32 pin |
|---|---|
| Pin 1 | 3V3 |
| Pin 2 (wiper) | GPIO4 |
| Pin 3 | GND |

> Reversed direction? Swap pins 1 and 3.
> GPIO4 is an ADC2 pin. It works fine here because this project doesn't use Wi-Fi or Bluetooth. If you add Wi-Fi later, move the pot to an ADC1 pin such as GPIO34 and change `POT_PIN` in the code.

### ESP32 → Relay Module (the 6 cylinders)

| Cylinder | Relay input | ESP32 pin |
|---|---|---|
| 1 | IN1 | GPIO16 |
| 2 | IN2 | GPIO17 |
| 3 | IN3 | GPIO18 |
| 4 | IN4 | GPIO19 |
| 5 | IN5 | GPIO21 |
| 6 | IN6 | GPIO22 |
| – | VCC | 5V (VIN) |
| – | GND | GND (common ground) |

---

## 🧩 Circuit Diagram

```
   47k POT (THROTTLE)         +-------------------------+
  .-------.                   |          ESP32          |
  | 1     |-------------------| 3V3                     |
  |       |                   |                         |
  | 2 (W) |---+---------------| GPIO4 (ADC2_CH0)        |
  |       |   |               |                         |
  | 3     |-+ = 0.1uF         |                         |
  '-------' | |               |                         |
           GND+---------------| GND --------------------+--------+
                              |                         |        |
                              | GPIO16 ---------------------> IN1 |  Cyl 1
                              | GPIO17 ---------------------> IN2 |  Cyl 2
                              | GPIO18 ---------------------> IN3 |  Cyl 3
                              | GPIO19 ---------------------> IN4 |  Cyl 4
                              | GPIO21 ---------------------> IN5 |  Cyl 5
                              | GPIO22 ---------------------> IN6 |  Cyl 6
                              | VIN (5V) -------------------> VCC |
                              +-------------------------+  GND ---+
                                                          6-CH RELAY
                                                          MODULE
                                           (relay contacts left EMPTY,
                                            only the click is used!)
```

---

## ⚙️ Working Mechanism

1. **Throttle input:** the pot works as a voltage divider (0–3.3 V). The ESP32's 12-bit ADC reads it as 0–4095.
2. **Smoothing:** 16 samples are averaged and filtered so the throttle feels like a real pedal without jitter.
3. **Throttle → Hz:** `f = 30 + (ADC/4095) × 60` gives 30 Hz (idle) to 90 Hz (full throttle).
4. **Engine timer:** the half period is `1,000,000 / (2f)` µs. A non-blocking `micros()` timer keeps timing accurate while the pot is read at the same time.
5. **Firing order:** in **V6 mode (MODE 1)** the relays fire one by one in order 1-4-2-5-3-6, so you get the lumpy V6 rhythm instead of a single buzz. In **MODE 0** all six fire together.
6. **Sound:** each relay clicks on fire and clicks again on release. At 30 Hz you hear a choppy idle, and at 90 Hz the clicks blur into a raspy engine scream.
7. **Dashboard:** the Serial Monitor shows throttle %, frequency and estimated RPM.

---

## 🚀 Getting Started

1. Install the Arduino IDE and add the **ESP32** board package (Espressif).
2. Open `Relay_to_V6_Engine.ino` (keep it inside a folder with the same name).
3. Select **ESP32 Dev Module** and your COM port, then click **Upload**.
4. Open Serial Monitor at **115200 baud**.
5. Twist the pot and **rev it!** 🏁

## 🎛️ Tuning

| Setting | Default | What it does |
|---|---|---|
| `MODE` | 1 | 0 = all relays together, 1 = V6 firing order |
| `FIRING_ORDER` | 1-4-2-5-3-6 | Change the order for a different engine rhythm |
| `FREQ_MIN` / `FREQ_MAX` | 30 / 90 | Idle and redline frequency |
| `SMOOTH` | 0.1 | Throttle response (lower = smoother) |
| `RELAY_ACTIVE_LOW` | false | Set `true` for active-LOW modules |

## 💡 Ideas to Make It Crazier

- Add a **rev limiter** bounce at redline
- Add a **start button** with a cranking sequence
- Add a button for **blow-off valve** pops
- Show RPM on an OLED display

---

## 📲 Follow the Build

Watch the build, hear the engine, and see more crazy projects:

- ▶️ **YouTube:** [@PandusTechLab](https://youtube.com/@pandustechlab?si=UCFYn3FYFlSZL5W3)
- 📸 **Instagram:** [@PandusTechLab]([YOUR_INSTAGRAM_LINK](https://www.instagram.com/pandutechlab/))

If you like it, **⭐ star the repo** and **subscribe**!

## 📜 License

MIT License. Build it, rev it, remix it.
