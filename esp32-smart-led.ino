#include <Preferences.h>
#include <IRremote.hpp>

/*
 * ESP32: 2 LED + button + IR remote state machine
 * Logic: 
 * - Main short-press cycle: OFF -> White 10% -> White 50% -> White 100% -> OFF.
 * - Long press: from ANY white mode -> Red; from Red -> White 10%.
 */

// ==========================================
// 1. PIN CONFIGURATION
// ==========================================
const int PIN_WHITE = 18;
const int PIN_RED   = 19;
const int PIN_BTN   = 23;
const int PIN_IR    = 4;

// ==========================================
// 2. PWM CONFIGURATION
// ==========================================
const int LEDC_CHANNEL = 0;
const int LEDC_FREQ    = 3000;
const int LEDC_RES     = 8;

// ==========================================
// 3. TIMING CONSTANTS
// ==========================================
const unsigned long DEBOUNCE_MS    = 30;
const unsigned long LONG_PRESS_MS  = 500;
const unsigned long CYCLE_TIMEOUT  = 5000;

// ==========================================
// 4. STATE MACHINE DEFINITION
// ==========================================
enum State {
  ST_OFF,
  ST_WHITE_10,
  ST_WHITE_50,
  ST_WHITE_100,
  ST_RED
};

State state = ST_OFF;

unsigned long lastPressTime   = 0;
unsigned long btnDownTime     = 0;
bool          btnDown         = false;
bool          longPressFired  = false;

Preferences prefs;
State last_active_color = ST_WHITE_10;

// ==========================================
// 5. IR REMOTE CODES (UPDATE THESE!)
// ==========================================
// Use ir_code_scanner.ino to find your remote's codes
const uint32_t IR_CODE_POWER = 0xBA45FF00;
const uint32_t IR_CODE_WHITE = 0xB946FF00;
const uint32_t IR_CODE_RED   = 0xB847FF00;

// ==========================================
// 6. HARDWARE CONTROL FUNCTIONS
// ==========================================

void setWhite(uint8_t duty) {
  ledcWrite(LEDC_CHANNEL, duty);
}

void setRed(bool on) {
  digitalWrite(PIN_RED, on ? HIGH : LOW);
}

void applyState() {
  switch (state) {
    case ST_OFF:
      setWhite(0);
      setRed(false);
      break;
    case ST_WHITE_10:
      setWhite(32);
      setRed(false);
      break;
    case ST_WHITE_50:
      setWhite(128);
      setRed(false);
      break;
    case ST_WHITE_100:
      setWhite(255);
      setRed(false);
      break;
    case ST_RED:
      setWhite(0);
      setRed(true);
      break;
  }

  if (state != ST_OFF) {
    last_active_color = state;
    prefs.putInt("last_color", (int)last_active_color);
  }
}

// ==========================================
// 7. PHYSICAL BUTTON HANDLERS
// ==========================================

void onShortPress() {
  unsigned long now = millis();

  if (state != ST_OFF && (now - lastPressTime) > CYCLE_TIMEOUT) {
    state = ST_OFF;
    lastPressTime = now;
    applyState();
    return;
  }

  switch (state) {
    case ST_OFF:
      if (last_active_color == ST_RED) {
        state = ST_RED;
      } else {
        state = ST_WHITE_10;
      }
      break;
    case ST_WHITE_10:  
      state = ST_WHITE_50; 
      break;
    case ST_WHITE_50:  
      state = ST_WHITE_100; 
      break;
    case ST_WHITE_100: 
      state = ST_OFF;       
      break;
    case ST_RED:       
      state = ST_OFF;       
      break;
  }
  lastPressTime = now;
  applyState();
}

void onLongPress() {
  unsigned long now = millis();
  
  if (state != ST_OFF && (now - lastPressTime) > CYCLE_TIMEOUT) {
    state = ST_OFF;
    lastPressTime = now;
    applyState();
    return;
  }

  // LONG PRESS LOGIC:
  // - From ANY white mode (10%, 50%, 100%) -> switch to Red
  // - From Red -> switch back to soft White 10%
  if (state == ST_WHITE_10 || state == ST_WHITE_50 || state == ST_WHITE_100) {
    state = ST_RED;
    lastPressTime = now;
    applyState();
  } else if (state == ST_RED) {
    state = ST_WHITE_10;
    lastPressTime = now;
    applyState();
  }
}

// ==========================================
// 8. IR REMOTE HANDLER
// ==========================================

void handleIRCommand(uint32_t code) {
  unsigned long now = millis();
  lastPressTime = now;

  switch (code) {
    case IR_CODE_POWER:
      if (state == ST_OFF) {
        state = (last_active_color == ST_RED) ? ST_RED : ST_WHITE_10;
      } else {
        state = ST_OFF;
      }
      break;

    case IR_CODE_WHITE:
      if (state == ST_OFF || state == ST_RED) {
        state = ST_WHITE_10;
      } else if (state == ST_WHITE_10) {
        state = ST_WHITE_50;
      } else if (state == ST_WHITE_50) {
        state = ST_WHITE_100;
      } else if (state == ST_WHITE_100) {
        state = ST_WHITE_10; 
      }
      break;

    case IR_CODE_RED:
      state = ST_RED;
      break;

    default:
      // Unknown codes are silently ignored in production
      break;
  }
  
  applyState();
}

// ==========================================
// 9. ARDUINO STANDARD FUNCTIONS
// ==========================================

void setup() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  ledcSetup(LEDC_CHANNEL, LEDC_FREQ, LEDC_RES);
  ledcAttachPin(PIN_WHITE, LEDC_CHANNEL);

  IrReceiver.begin(PIN_IR);

  prefs.begin("led_ctrl", false);
  last_active_color = (State)prefs.getInt("last_color", ST_WHITE_10);
  
  state = ST_OFF; 
  applyState();
}

void loop() {
  unsigned long now = millis();

  // 1. Process IR Remote Input
  if (IrReceiver.decode()) {
    uint32_t code = IrReceiver.decodedIRData.decodedRawData;
    if (code != 0) {
      handleIRCommand(code);
    }
    IrReceiver.resume(); 
  }

  // 2. Process Physical Button Input
  bool pressed = (digitalRead(PIN_BTN) == LOW); 

  if (pressed) {
    if (!btnDown) {
      btnDown = true;
      btnDownTime = now;
      longPressFired = false;
    } else {
      if (!longPressFired && (now - btnDownTime) >= LONG_PRESS_MS) {
        longPressFired = true; 
        onLongPress();
      }
    }
  } else {
    if (btnDown) {
      unsigned long held = now - btnDownTime;
      if (!longPressFired && held >= DEBOUNCE_MS) {
        onShortPress();
      }
      btnDown = false;
      longPressFired = false;
    }
  }
}
