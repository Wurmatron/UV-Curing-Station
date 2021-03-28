// Libs
#include "Wire.h"
#include "LCD.h"
#include "LiquidCrystal_I2C.h"
#include <Stepper.h>

// Pinout
const int stepper_in1 = 10;
const int stepper_in2 = 11;
const int stepper_in3 = 12;
const int stepper_in4 = 13;
// const int lcd_sda = A5;
// const int lcd_scl = A4;
const int button_stop = 2;
const int button_leftA = 4;
const int button_leftB = 7;
const int button_rightA = 8;
const int button_leftB = 9;
const int led_red = 3;
const int led_blue = 5;
const int led_green = 6;

// Const
const int stepsPerRevolution = 2048;
const int stepperSpeed = 5;

long timeLeft = 0;
int stepperSpin = 0;
int uvEnabled = 0;

// Devices
LiquidCrystal_I2C lcd(0x3F,20,4); // 0x27
Stepper stepper = Stepper(stepsPerRevolution, stepper_in1, stepper_in3, stepper_in2, stepper_in4);

void setup() {
  // LCD setup
  lcd.init();
  lcd.backlight();
  // Button setup
  pinMode(button_stop, INPUT);
  pinMode(button_leftA, INPUT);
  pinMode(button_leftB, INPUT);
  pinMode(button_rightA, INPUT);
  pinMode(button_rightB, INPUT);
  // LED setup
  pinMode(led_red, OUTPUT);
  pinMode(led_blue, OUTPUT);
  pinMode(led_green, OUTPUT);
  // Stepper setup
  stepper.setSpeed(stepperSpeed);
  // Coms
  Serial.begin(9600);
  lcd.setCursor(0,0);
  lcd.print("Starting...");
}

void loop() {
    updateStepper();
    displayLCD();
    handleButtonInteractions();
}

void handleButtonInteractions() {
  if(buttonPressed())
}

// Keeps the stepper spinning
void updateStepper() {
  if(timeLeft > 0 && stepperSpin == 1) {  // Spin if a timer is running
    stepper.step(stepsPerRevolution);
  }
}

void displayLCD() {
  int t = timeLeft / 1000;
  displayMenu(t);
}

void displayMenu(int time) {
  wipeScreen();
  lcd.setCursor(0,0);
  lcd.print("Time: " + String(time));
  lcd.setCursor(0,1);
  if(timeLeft > 0) {
    lcd.print("Running...");
  }
}

void wipeScreen() {
  int x;
  for(x = 0; x < 1; x++) {
    lcd.setCursor(0,x);
    lcd.print("                ");
  }
  free(x);
}

int buttonPressed(int buttonPin) {
  static uint16_t lastStates = 0;
  int state = digitalRead(buttonPin);
  if (state != ((lastStates >> buttonPin) & 1)) {
    lastStates ^= 1 << buttonPin;
    return state == HIGH;
  }
  return false;
}
