#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "FLAT-6-2G";
const char* password = "ptpassword";
const char* FIRMWARE_SERVER_IP_ADDR = "143.167.81.144"; // Change this to your server's IP address
const int FIRMWARE_SERVER_PORT = 8000; // Change the port if needed

int firmwareVersion = 2; // Increment this value and recompile the sketch for each new version

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to Wi-Fi");
  performOTAUpdate();
}

void loop() {
  // Your main code here
}

void performOTAUpdate() {
  HTTPClient http;
  char url[256];

  snprintf(url, sizeof(url), "http://%s:%d/version.txt", FIRMWARE_SERVER_IP_ADDR, FIRMWARE_SERVER_PORT);
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    int latestVersion = atoi(http.getString().c_str());
    if (latestVersion > firmwareVersion) {
      snprintf(url, sizeof(url), "http://%s:%d/firmware_%d.bin", FIRMWARE_SERVER_IP_ADDR, FIRMWARE_SERVER_PORT, latestVersion);
      http.begin(url);
      int httpCode = http.GET();

      if (httpCode == 200) {
        Serial.println("Starting OTA update...");
        if (Update.begin(http.getSize())) {
          size_t written = Update.writeStream(*http.getStreamPtr());
          if (written == http.getSize()) {
            Serial.println("Written : " + String(written) + " successfully");
          } else {
            Serial.println("Written only : " + String(written) + "/" + String(http.getSize()) + ". Retry?" );
          }
          if (Update.end()) {
            Serial.println("OTA update complete. Rebooting...");
            ESP.restart();
          } else {
            Serial.println("Error Occurred. Error #: " + String(Update.getError()));
          }
        } else {
          Serial.println("Not enough space to begin OTA");
        }
      } else {
        Serial.printf("Unable to fetch firmware: HTTP code %d\n", httpCode);
      }
    } else {
      Serial.println("Firmware is up to date");
    }
  } else {
    Serial.printf("Unable to fetch version.txt: HTTP code %d\n", httpCode);
  }
  http.end();
}
