// sketch.ino
// main entry points

#include <Arduino.h>
#include <esp_log.h>

// to blink or not to blink...
bool doBlinking = true;

void setup() {
  Serial.begin(115200);
  Serial.println("arduino started");
  pinMode(LED_BUILTIN, OUTPUT);
} // setup

void loop() {
  Serial.printf("\nahem, hello world\n");

  #ifdef ESP_IDF_VERSION_MAJOR
    Serial.printf( // IDF version
      "IDF version: %d.%d.%d\n",
      ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH
    );
  #endif
  #ifdef ESP_ARDUINO_VERSION_MAJOR
    Serial.printf(
      "ESP_ARDUINO_VERSION_MAJOR=%d; MINOR=%d; PATCH=%d\n",
      ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR,
      ESP_ARDUINO_VERSION_PATCH
    );
  #endif
  #ifdef ARDUINO_FEATHER_ESP32
    Serial.printf("ARDUINO_FEATHER_ESP32 is defined\n");
  #endif
  #ifdef ARDUINO_ARCH_ESP32
    Serial.printf("ARDUINO_ARCH_ESP32 is defined\n");
  #endif
  #ifdef ESP_PLATFORM
    Serial.printf("ESP_PLATFORM is defined\n");
  #endif
  #ifdef ESP32
    Serial.printf("ESP32 is defined\n");
  #endif
  #ifdef IDF_VER
    Serial.printf("IDF_VER=%s\n", IDF_VER);
  #endif
  #ifdef ARDUINO
    Serial.printf("ARDUINO=%d\n", ARDUINO);
  #endif
  #ifdef ARDUINO_BOARD
    Serial.printf("ARDUINO_BOARD=%s\n", ARDUINO_BOARD);
  #endif
  #ifdef ARDUINO_VARIANT
    Serial.printf("ARDUINO_VARIANT=%s\n", ARDUINO_VARIANT);
  #endif
  #ifdef ARDUINO_SERIAL_PORT
    Serial.printf("ARDUINO_SERIAL_PORT=%d\n", ARDUINO_SERIAL_PORT);
  #endif

  if(doBlinking) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(2000);
  }
} // loop
