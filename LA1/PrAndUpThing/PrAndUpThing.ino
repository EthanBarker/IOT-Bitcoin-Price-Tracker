// PrAndUpThing.ino
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "Ethan's ESP32 Access Point"; // SSID of the ESP32 access point
const char* password = "password"; // Password for the access point

#define FIRMWARE_SERVER_IP_ADDR "10.254.7.164"
#define FIRMWARE_SERVER_PORT    8000

int firmwareVersion = 2; // Increment this value and recompile the sketch for each new version

WebServer server(80); // Create a web server on port 80

// Define GPIO pins for LEDs
const int red = 6; // Provisioning
const int yellow = 9; // Updating
const int green = 12; // Completed

// Define GPIO pin for the button
const int buttonPin = 5;



void handleRoot() {
  String page = "<html><body><h1>Available Wi-Fi Networks</h1><ul>";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    page += "<li>";
    page += WiFi.SSID(i);
    if (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) {
      page += " (password protected)";
    }
    page += "  <a href=\"/connect?ssid=";
    page += WiFi.SSID(i);
    page += "\">Connect</a>";
    page += "</li>";
  }
  page += "</ul></body></html>";
  server.send(200, "text/html", page);
}

void handleConnect() {
  String ssid = server.arg("ssid");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  String key = "";
  if (server.hasArg("key")) {
    key = server.arg("key");
  } else {
    String page = "<html><body><h1>Enter Wi-Fi Password</h1>";
    page += "<form method='get' action='/connect'>";
    page += "<label for='key'>Password:</label>";
    page += "<input type='password' id='key' name='key'><br><br>";
    page += "<input type='submit' value='Connect'>";
    page += "<input type='hidden' name='ssid' value='" + ssid + "'>";
    page += "</form></body></html>";
    server.send(200, "text/html", page);
    return;
  }

  WiFi.begin(ssid.c_str(), key.c_str());
  digitalWrite(red, HIGH); // Turn on the red LED while connecting
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  
  digitalWrite(red, LOW);
  digitalWrite(green, HIGH); // Turn on the green LED once connected
  Serial.println("Connected to Wi-Fi.");
  delay(10000);
  digitalWrite(green, LOW); //Turn off the Green LED after 1000ms
  
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  server.send(200, "text/html", "<html><body><h1>Connected to " + ssid + "</h1><p><a href='/'>Go back to Home Page</a></p></body></html>");
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

void setup() {
  // Wait for 15 seconds before executing the rest of the code
  delay(15000);

  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(red, HIGH); // Initialize the red LED to be on
  
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  server.on("/", handleRoot);
  server.on("/connect", handleConnect);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();

  if (digitalRead(buttonPin) == LOW) {
    performOTAUpdate();
    delay(300); // Add a small delay to debounce the button
  }
}

void performOTAUpdate() {
  digitalWrite(yellow, HIGH);
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
            digitalWrite(green, HIGH); // Turn on the green LED
            digitalWrite(yellow, LOW); // Turn off the yellow LED
            delay(10000); // Wait for 10 seconds
            digitalWrite(green, LOW); // Turn off the green LED
          } else {
            Serial.println("Error Occurred. Error #: " + String(Update.getError()));
            digitalWrite(red, HIGH); // Turn on the red LED
            digitalWrite(yellow, LOW); // Turn off the yellow LED
            delay(10000); // Wait for 10 seconds
            digitalWrite(red, LOW); // Turn off the red LED
          }
        } else {
          Serial.println("Not enough space to begin OTA");
          digitalWrite(red, HIGH); // Turn on the red LED
          digitalWrite(yellow, LOW); // Turn off the yellow LED
          delay(10000); // Wait for 10 seconds
          digitalWrite(red, LOW); // Turn off the red LED
        }
      } else {
        Serial.printf("Unable to fetch firmware: HTTP code %d\n", httpCode);
        digitalWrite(red, HIGH); // Turn on the red LED
        digitalWrite(yellow, LOW); // Turn off the yellow LED
        delay(10000); // Wait for 10 seconds
        digitalWrite(red, LOW); // Turn off the red LED
      }
    } else {
      Serial.println("Firmware is up to date");
      digitalWrite(red, HIGH); // Turn on the red LED
      digitalWrite(yellow, LOW); // Turn off the yellow LED
      delay(10000); // Wait for 10 seconds
      digitalWrite(red, LOW); // Turn off the red LED
    }
  } else {
    Serial.printf("Unable to fetch version.txt: HTTP code %d\n", httpCode);
    digitalWrite(red, HIGH); // Turn on the red LED
    digitalWrite(yellow, LOW); // Turn off the yellow LED
    delay(10000); // Wait for 10 seconds
    digitalWrite(red, LOW); // Turn off the red LED
  }
  http.end();
}
