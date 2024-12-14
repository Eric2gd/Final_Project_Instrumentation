#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize the LCD with I2C address (usually 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define input pins for op-amp outputs
const int opAmp1Pin = 2;  // Op-amp 1 output
const int opAmp2Pin = 3;  // Op-amp 2 output

volatile int transitionCount = 0; // Counts transitions (HIGH->LOW or LOW->HIGH)
unsigned long previousMillis = 0; // Tracks the last time RPM was calculated
const int interval = 1000;        // Interval for RPM calculation in milliseconds (1 second)
float rpm = 0;                    // Holds the calculated RPM
bool lastState = LOW;             // Keeps track of the previous state of the pin

// Define output pins
 // Additional output pin
const int motorControlPin = 9;  // Pin connected to motor driver's input

void setup() {
  // Set up the pins
  pinMode(opAmp1Pin, INPUT);
  pinMode(opAmp2Pin, INPUT);
  pinMode(motorControlPin, OUTPUT);
  pinMode(6, INPUT_PULLUP); // Use INPUT_PULLUP to ensure a stable signal

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  // Initialize Serial Monitor
  Serial.begin(9600);

  // Display a startup message
  lcd.setCursor(0, 0);
  lcd.print("Temp Monitor");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read the op-amp outputs
  digitalWrite(opAmp1Pin,LOW);
  digitalWrite(opAmp2Pin,HIGH);
  int opAmp1State = digitalRead(opAmp1Pin);  // Read opAmp1Pin state
  int opAmp2State = digitalRead(opAmp2Pin);  // Read opAmp2Pin state
  unsigned long currentMillis = millis();

  // Determine the status based on op-amp states
  if (opAmp1State == HIGH && opAmp2State == LOW) {
    // Temperature below range
    Serial.println("Temperature below range");
    displayMessage("Below Range");
    analogWrite(motorControlPin, 0);  // Stop the motor
  } else if (opAmp1State == LOW && opAmp2State == HIGH) {
    // Temperature above range
    Serial.println("Fan on");
    Serial.println("Temperature above range");
    displayMessage("Above Range");
    if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Calculate RPM: Two transitions equal one revolution
    rpm = (transitionCount / 2.0) * (60000.0 / interval); // 60000 ms in a minute
    transitionCount = 0; // Reset the transition count for the next interval

    // Print RPM to the serial monitor
    Serial.print("Fan Speed: ");
    Serial.print(rpm);
    Serial.println(" RPM");
    displayMessage("Above Range");
    displayMessage2(rpm);
  }
    analogWrite(motorControlPin, 255);  // Spin the motor
  } else if (opAmp1State == HIGH && opAmp2State == HIGH) {
    // Temperature within range
    Serial.println("Temperature within range");
    displayMessage("Within Range");
    analogWrite(motorControlPin, 0);   // Stop the motor
  } else {
    // Invalid state or error
    Serial.println("Invalid op-amp outputs");
    displayMessage("Error: Invalid");
    analogWrite(motorControlPin, 0); ;  // Stop the motor
  }

  delay(1000);  // Update the display every second
}

// Function to display a message on the LCD
void displayMessage(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.setCursor(3, 0);
  lcd.print(message);
}

void displayMessage2(float message) {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.setCursor(7 , 6);
  lcd.print(message);
}
