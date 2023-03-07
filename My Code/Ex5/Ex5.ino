#define LED1_PIN 2
#define LED2_PIN 3
#define LED3_PIN 4

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
}

void loop() {
  blink(LED1_PIN, 5);
  blink(LED2_PIN, 2);
  blink(LED3_PIN, 4);
  blink(LED2_PIN, 1);
  blink(LED1_PIN, 5);
}

void blink(int pin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(250);
    digitalWrite(pin, LOW);
    delay(250);
  }
}
