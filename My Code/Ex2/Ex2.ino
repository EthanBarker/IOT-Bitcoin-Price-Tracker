int buttonPin = 18;  // the pin that the button is connected to
int ledPin = 19;     // the pin that the LED is connected to

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // set up the button pin as an input with a pullup resistor
  pinMode(ledPin, OUTPUT);           // set up the LED pin as an output
  Serial.begin(115200);              // initialize the serial line
}

void loop() {
  int buttonState = digitalRead(buttonPin);  // read the state of the button
  if (buttonState == LOW) {                  // if the button is pressed
    digitalWrite(ledPin, HIGH);              // turn on the LED
    Serial.println("Button pressed!");       // print a message on the serial line
    delay(100);                              // wait for 100ms to debounce the button
  } else {                                   // if the button is not pressed
    digitalWrite(ledPin, LOW);               // turn off the LED
  }
}
