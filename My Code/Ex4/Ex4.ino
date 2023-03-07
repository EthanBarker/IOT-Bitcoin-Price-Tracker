#define DEBUG 1 // Set this to 0 to turn off debug mode
#define debugPrintLn(x) if(DEBUG) { Serial.println(x); } // Define debug print macro

void setup() {
  Serial.begin(9600);
  debugPrintLn("Debugging infrastructure is ready");
}

void loop() {
  debugPrintLn("Entering loop");
  
  // Code for LED 1
  debugPrintLn("Turning on LED 1");
  digitalWrite(LED1_PIN, HIGH);
  delay(5000);
  digitalWrite(LED1_PIN, LOW);
  
  // Code for LED 2
  debugPrintLn("Turning on LED 2");
  digitalWrite(LED2_PIN, HIGH);
  delay(5000);
  digitalWrite(LED2_PIN, LOW);
  
  // Code for LED 3
  debugPrintLn("Turning on LED 3");
  digitalWrite(LED3_PIN, HIGH);
  delay(5000);
  digitalWrite(LED3_PIN, LOW);
}
