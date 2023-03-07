int ledPin1 = 19;   // the pin that the first LED is connected to
int ledPin2 = 21;   // the pin that the second LED is connected to
int ledPin3 = 23;   // the pin that the third LED is connected to

void setup() {
  pinMode(ledPin1, OUTPUT);   // set up the first LED pin as an output
  pinMode(ledPin2, OUTPUT);   // set up the second LED pin as an output
  pinMode(ledPin3, OUTPUT);   // set up the third LED pin as an output
  Serial.begin(115200);       // initialize the serial line
}

void loop() {
  digitalWrite(ledPin1, HIGH);   // turn on the first LED
  Serial.println("First LED on"); // print a message on the serial line
  delay(5000);                   // wait for 5 seconds
  digitalWrite(ledPin1, LOW);    // turn off the first LED

  digitalWrite(ledPin2, HIGH);   // turn on the second LED
  Serial.println("Second LED on");// print a message on the serial line
  delay(5000);                   // wait for 5 seconds
  digitalWrite(ledPin2, LOW);    // turn off the second LED

  digitalWrite(ledPin3, HIGH);   // turn on the third LED
  Serial.println("Third LED on"); // print a message on the serial line
  delay(5000);                   // wait for 5 seconds
  digitalWrite(ledPin3, LOW);    // turn off the third LED
}
