// PrAndUpThing.ino
#include <WiFi.h>
#include <Wire.h>

// Wi-Fi credentials
const char* ssid = "FLAT-6-2G";
const char* password = "ptpassword";

// Define GPIO pins for LEDs
const int red = 6; // Provisioning
const int yellow = 9; // Updating
const int green = 12; // Completed

// Define GPIO pin for the button
const int buttonPin = 5;

void setup() {
  Serial.begin(115200);

  // Wait for 15 seconds before executing the rest of the code
  delay(15000);

  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // Connect to Wi-Fi
  connectWiFi();

  // Mock provisioning
  mockProvisioning();
}

void loop() {
  // Check if the button is pressed
  if (digitalRead(buttonPin) == LOW) {
    // Mock update process
    mockUpdate();
    delay(300); // Add a small delay to debounce the button
  }
}

void connectWiFi() {
  digitalWrite(red, HIGH);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  digitalWrite(red, LOW);
  Serial.println("Connected to Wi-Fi.");
}

void mockProvisioning() {
  digitalWrite(red, HIGH);
  Serial.println("Provisioning...");

  // Mock provisioning process
  delay(2000);

  digitalWrite(red, LOW);
  Serial.println("Provisioning completed.");
}

void mockUpdate() {
  digitalWrite(yellow, HIGH);
  Serial.println("Updating...");

  // Mock update process
  delay(2000);

  digitalWrite(yellow, LOW);
  Serial.println("Update completed.");

  // Indicate completion with the green LED
  digitalWrite(green, HIGH);
  delay(1000);
  digitalWrite(green, LOW);
}
