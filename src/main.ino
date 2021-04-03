/**
*	UV Curing Station
*	created by Wurmatron
**/
// Libs
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include <avr/sleep.h>

// Pinout
const int uvGate = 10;    // N-Channel Mosfet
const int motorGate = 11; // N-Channel Mosfet
// const int lcd_sda = A5;
// const int lcd_scl = A4;
const int button_stop = 2;
const int button_leftA = 4;
const int button_leftB = 7;
const int button_rightA = 8;
const int button_rightB = 9;
const int led_red = 3;
const int led_blue = 5;
const int led_green = 6;

// Devices
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F,20,4); // 0x27

// Defaults
const long presets[] = {3 * 60000, 5 * 60000, 10 * 60000, 15 * 60000};  // Time in MS
const int presetLen = 4;  // Amount of presets
const long pollRate = 20;   // How often to update everything
const int lcdUpdateRate = 10; // pollRate * lcdUpdateRate is time in ms
const long errorTime = 2000; // How long an error should be displayed for
const long timerStepIncrement = 5000;  // 5 seconds
const long inactiveSleep = (30 * 1000) / pollRate; //  30 sec
const int displayInactive = (10 * 1000) / pollRate; // 10 sec

// Current vals
int running = 0;
long currentTime = 0;
int currentPreset = 0;
int errorTimeout = 0;
// Temp vars
long secs = 0;
long min = 0;
long sec = 0;
int lcdUpdate = 0;
long inactiveCount = 0;
int isSleeping = 0;

// Configure Pins, LCD, Serial
void setup() {
  // LCD setup
  lcd.begin();
  // Button setup
  pinMode(button_stop, INPUT_PULLUP);
  pinMode(button_leftA, INPUT_PULLUP);
  pinMode(button_leftB, INPUT_PULLUP);
  pinMode(button_rightA, INPUT_PULLUP);
  pinMode(button_rightB, INPUT_PULLUP);
  // LED setup
  pinMode(led_red, OUTPUT);
  pinMode(led_blue, OUTPUT);
  pinMode(led_green, OUTPUT);
  // Gate setup
  pinMode(uvGate, OUTPUT);
  pinMode(motorGate, OUTPUT);
  // Coms
  Serial.begin(9600);
  // Info setup
  lcd.setCursor(0,0);
  lcd.print("Starting...");
  digitalWrite(led_blue, HIGH);
}

// Main CPU Loop
void loop() {
  if(buttonPressed(button_stop))
    toggleRunning();

  // Button Logic
  if(!running) {
    handleRightButtons();
    handleLeftButtons();
  }

  // Station Logic
  handleMosfet(uvGate);
  handleMosfet(motorGate);
  checkForSleep();

  // Display Logic
  updateLEDStatus();
  if(errorTimeout == 0) {
    updateTimer();
    updateLEDStatus();
    if(running) {
      currentTime -= pollRate;
      if(currentTime < 0) // Prevent Below 0
       currentTime = 0;
      if(currentTime == 0) { // Pause once completed
        running = false;
        digitalWrite(led_blue, HIGH);
      }
    } else {
        if(!isSleeping)
          inactiveCount += 1;
        if(inactiveCount > displayInactive) {
          lcd.clear();
          lcd.print("Inactive: " + String(inactiveCount/(1000 / pollRate)));
        }
    }
  } else {
    errorTimeout -= pollRate;
  }
  // Wait for pollRate
  delay(pollRate);
}

// Check if a given button has been pressed
int buttonPressed(uint8_t button) {
  static uint16_t lastStates = 0;
  uint8_t state = digitalRead(button);
  if (state != ((lastStates >> button) & 1)) {
    lastStates ^= 1 << button;
    if(state == HIGH)
       inactiveCount = 0;
    return state == HIGH;
  }
  return false;
}

// Enabled / Disabled the timer
void toggleRunning() {
  running = !running;
  if(running && currentTime <= 0) {
    lcd.print("Time must be");
    lcd.setCursor(0,1);
    lcd.print("greater than 0");
    digitalWrite(led_red, HIGH);
    errorTimeout = errorTime;
  }
}

// Handles the right side of the panel (Add, Sub) from timer
void handleRightButtons() {
  if(buttonPressed(button_rightA)) { // Sub to timer
      currentTime -= timerStepIncrement;
  }
  if(buttonPressed(button_rightB)) { // Add from timer
     currentTime += timerStepIncrement;
  }
}

// Handles the left side of the panel (Left, Right) Schedule for timer presets
void handleLeftButtons() {
  if(buttonPressed(button_leftA)) { // "Scroll" Left
      currentPreset -= 1;
      // Scrolling Effect
      if(currentPreset < 0) {
        currentPreset = presetLen;
      }
      currentTime = presets[currentPreset];
  }
  if(buttonPressed(button_leftB)) { // "Scroll" Right
    currentPreset += 1;
    // Scrolling Effect
    if(currentPreset > presetLen) {
      currentPreset = 0;
    }
    currentTime = presets[currentPreset];
  }
}

// Update LCD for current timer and status
void updateTimer() {
  if(lcdUpdate == 0) {
    lcd.clear();
    lcd.setCursor(0,0);
    secs = (currentTime / 1000);
    min = secs / 60;
    sec = secs - (min * 60);
    lcd.print(String(min) + ":" + String(sec) + " Left");
    lcd.setCursor(0,1);
    if(running)
      lcd.print("Running...");
    else
      lcd.print("Paused");
    // Reset lcdUpdate
    lcdUpdate = lcdUpdateRate;
  } else {
    lcdUpdate -= 1;
  }
}

// Update LED Status based on current program state
void updateLEDStatus() {
  if(errorTimeout == 0) // Only If not errored
    if(running) { // Is Running
      digitalWrite(led_red, LOW);
      digitalWrite(led_green, HIGH);
      digitalWrite(led_blue, LOW);
    } else {  // Not Running
      if(!isSleeping) {
        digitalWrite(led_red, HIGH);
        digitalWrite(led_green, HIGH);
        digitalWrite(led_blue, LOW);
      }
    }
  if(errorTimeout > 0) {  // Had an error, previously
    digitalWrite(led_red, HIGH);
    digitalWrite(led_green, LOW);
    digitalWrite(led_blue, LOW);
  }
}
 
void handleMosfet(int pin) {
  if(running && currentTime > 0)
    digitalWrite(pin, HIGH);
  else
    digitalWrite(pin, LOW);
}

void checkForSleep() {
  if(inactiveCount > inactiveSleep && !running) {
    lcd.noBacklight();
    digitalWrite(led_red, LOW);
    digitalWrite(led_green, LOW);
    digitalWrite(led_blue, LOW);
    isSleeping = 1;
  } else {
     lcd.backlight();
     isSleeping = 0;
  }
}
