// Ex03.cpp/.ino
// add more LEDs and switches; run as traffic lights or individually

#include "Thing.h"

const int red = 6; // LEDs are assigned to pins 6, 9 and 12
const int yellow = 9;
const int green = 12;
bool ON_RED = false;
void changeLights();

void setup03() {
  setup02();                      // previous setups...
  Serial.println("\nsetup03..."); // debug printout

  // set up further GPIO pins with nice names for additional external LEDs
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  digitalWrite(green, HIGH); // we start the traffic light sequence on green
}

void loop03() {
  if (digitalRead(5) == LOW) { // if the switch is LOW, ie. pushed down then
    Serial.printf(" ON_RED is %d; switch pressed...\n", ON_RED);

    changeLights();             // call this function to change the lights!
  }
}

void changeLights() {
  if(ON_RED) {
    // red and yellow on for 2 seconds (red is already on though)
    digitalWrite(yellow, HIGH);
    delay(2000);

    // turn off red and yellow, then turn on green
    digitalWrite(yellow, LOW);
    digitalWrite(red, LOW);
    digitalWrite(green, HIGH);
  } else {
    // green off, yellow on for 3 seconds
    digitalWrite(green, LOW);
    digitalWrite(yellow, HIGH);
    delay(3000);

    // turn off yellow, then turn red on for 5 seconds
    digitalWrite(yellow, LOW);
    digitalWrite(red, HIGH);
  }

  ON_RED = ! ON_RED;
}
