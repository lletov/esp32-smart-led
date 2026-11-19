# 💡 ESP32 Smart LED Controller (IR + Button)

A lightweight, production-ready state-machine-based LED controller for ESP32. It provides dual control (physical button + IR remote) of a dual-color LED setup, featuring a linear white-color cycle, a dedicated red side-branch, non-volatile state memory, and an inactivity auto-off timeout.

The main firmware runs fully autonomously — no PC connection required after initial setup.

## ✨ Features

- **Dual Control:** Seamless operation via a physical button or any standard IR remote.
- **Linear White Cycle:** Short presses cycle smoothly through: `OFF` → `White 10%` → `White 50%` → `White 100%` → `OFF`.
- **Color Toggle Shortcut:** Long press instantly switches to **Red** from *any* white mode, and back to **White 10%** from Red.
- **State Memory:** Remembers the last active color/brightness using ESP32 NVS (Preferences) and resumes intelligently on power-up.
- **Inactivity Timeout:** Automatically turns off after 5 seconds of inactivity to save power.
- **Standalone Ready:** Main firmware has no Serial output — ideal for embedded deployment.


## 🛠 Hardware Requirements

- ESP32 Development Board (any DevKit variant)
- 1× White LED (with ~220Ω current-limiting resistor)
- 1× Red LED (with ~220Ω current-limiting resistor)
- 1× Tactile push button
- 1× IR receiver module (e.g., VS1838B or KY-022)
- Breadboard and jumper wires
- TODO

## 🔌 Wiring Diagram

| Component | ESP32 Pin | Notes |
| :--- | :---: | :--- |
| White LED (anode via resistor) | GPIO 18 | Must be a PWM-capable pin |
| Red LED (anode via resistor) | GPIO 19 | Digital output only |
| Push button | GPIO 23 | Other leg to GND (uses internal pull-up) |
| IR receiver (Signal / OUT) | GPIO 4 | VCC → 3.3V / 5V, GND → GND |

## ⚙️ Setup & Configuration

### Step 1 — Install Dependencies

1. Install the Arduino IDE and add ESP32 board support via the Boards Manager.
2. Install the required IR library:  
   **Tools → Manage Libraries…** → search for `IRremote` → install the version by *Armin Joachimsmeyer*.

### Step 2 — Map Your IR Remote Codes

Every IR remote uses unique hex codes, so you need to scan yours once:

1. Open and upload `ir-code-scanner.ino` to your ESP32.
2. Open the **Serial Monitor** in Arduino IDE and set the baud rate to **115200**.
3. Press each button on your remote which should be used (`POWER`, `WHITE`, `RED`) and copy the displayed hex codes.

### Step 3 — Configure the Main Firmware

1. Open `esp32-smart-led.ino`.
2. Replace the placeholder IR codes at the top of the file with your actual codes:

   ```cpp
   const uint32_t IR_CODE_POWER = 0xYOUR_CODE_HERE;
   const uint32_t IR_CODE_WHITE = 0xYOUR_CODE_HERE;
   const uint32_t IR_CODE_RED   = 0xYOUR_CODE_HERE;
   ```
3. Upload `esp32-smart-led.ino` to your ESP32.
4. Disconnect from PC — the device is now ready for autonomous operation.

## 🎮 Usage Guide
### Physical Button

| Action | From (State) | To (Result State) |
| :--- | :--- | :--- |
| Short press | OFF | White 10% (or Red if it was the last active state) |
| Short press | White 10% | White 50% |
| Short press | White 50% | White 100% |
| Short press | White 100% | OFF |
| Short press | Red | OFF |
| Long press (>0.5s) | Any white mode | Red |
| Long press (>0.5s) | Red | White 10% |

### IR Remote


| Button | Behavior |
| :--- | :--- |
| **POWER** | Toggles ON / OFF (resumes last active color) |
| **WHITE** | Forces White mode immediately, cycles exclusively through white brightness: 10% → 50% → 100% → 10% |
| **RED** | Forces Red mode immediately |
