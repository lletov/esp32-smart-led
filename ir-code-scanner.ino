/*
 * IR Code Scanner Utility
 * 
 * This sketch is used ONCE during setup to identify the hex codes
 * of your IR remote buttons. After finding the codes, update them
 * in esp32_led_controller.ino and upload that sketch for production use.
 * 
 * Usage:
 * 1. Upload this sketch to your ESP32
 * 2. Open Serial Monitor at 115200 baud
 * 3. Press buttons on your remote
 * 4. Copy the displayed hex codes
 * 5. Paste them into esp32_led_controller.ino
 */

#include <IRremote.hpp>

const int PIN_IR = 4; // IR Receiver signal pin

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("====================================");
  Serial.println("   IR CODE SCANNER UTILITY");
  Serial.println("====================================");
  Serial.println();
  Serial.println("Press any button on your IR remote.");
  Serial.println("The hex code will be displayed below.");
  Serial.println();
  Serial.println("Instructions:");
  Serial.println("1. Press POWER button -> copy code -> paste to IR_CODE_POWER");
  Serial.println("2. Press WHITE button -> copy code -> paste to IR_CODE_WHITE");
  Serial.println("3. Press RED button   -> copy code -> paste to IR_CODE_RED");
  Serial.println();
  Serial.println("====================================");
  Serial.println();
  
  IrReceiver.begin(PIN_IR);
}

void loop() {
  if (IrReceiver.decode()) {
    uint32_t code = IrReceiver.decodedIRData.decodedRawData;
    
    // Ignore repeat signals (code = 0)
    if (code != 0) {
      Serial.print("Received IR code: 0x");
      Serial.println(code, HEX);
      Serial.println();
      Serial.println("Copy this value and paste it into esp32_led_controller.ino");
      Serial.println("------------------------------------");
    }
    
    IrReceiver.resume();
    delay(200); // Small delay to prevent flooding
  }
}
