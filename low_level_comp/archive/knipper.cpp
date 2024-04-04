#include <Arduino.h>

// Define the pin connected to the LED
#define LED_PIN 3 // PB3, physical pin 2 on ATtiny85

void setup() {
  // Initialize the digital pin as an output
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Turn on the LED
  digitalWrite(LED_PIN, HIGH);
  delay(1000); // Wait for 1 second

  // Turn off the LED
  digitalWrite(LED_PIN, LOW);
  delay(1000); // Wait for 1 second
}
