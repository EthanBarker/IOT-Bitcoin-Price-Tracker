// PrAndUpThing.ino
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "Ethan's ESP32 Access Point"; // SSID of the ESP32 access point
const char* password = "password"; // Password for the access point

#define FIRMWARE_SERVER_IP_ADDR "10.254.7.164"
#define FIRMWARE_SERVER_PORT    8000

int firmwareVersion = 1; // Increment this value and recompile the sketch for each new version

WebServer server(80); // Create a web server on port 80

// Define GPIO pins for LEDs
const int red = 6; // Provisioning
const int yellow = 9; // Updating
const int green = 12; // Completed

// Define GPIO pin for the button
const int buttonPin = 5;

void handleRoot() {
  String page = "<html><body><h1>Available Wi-Fi Networks</h1><ul>";
  // Scan wifi networks that are available and iterate through a list
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    // Add each network to the HTML list
    page += "<li>";
    page += WiFi.SSID(i);
    // Check if the network is password protected 
    if (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) {
      page += " (password protected)";
    }
    // Add a connect button for each network
    page += "  <a href=\"/connect?ssid=";
    page += WiFi.SSID(i);
    page += "\">Connect</a>";
    page += "</li>";
  }
  page += "</ul></body></html>";
  // Send the HTML page as a responce of the GET request
  server.send(200, "text/html", page);
}

void handleConnect() {
  // Get SSID and password from GET request
  String ssid = server.arg("ssid");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Check if a password was provided or ask for one using HTML 
  String key = "";
  if (server.hasArg("key")) {
    key = server.arg("key");
  } else {
    // Prompt user for a password
    String page = "<html><body><h1>Enter Wi-Fi Password</h1>";
    page += "<form method='get' action='/connect'>";
    page += "<label for='key'>Password:</label>";
    page += "<input type='password' id='key' name='key'><br><br>";
    page += "<input type='submit' value='Connect'>";
    page += "<input type='hidden' name='ssid' value='" + ssid + "'>";
    page += "</form></body></html>";
    // Send the HTML as a responce to the GET request
    server.send(200, "text/html", page);
    return;
  }
  // Connect to the Wifi network using SSID and passwod
  WiFi.begin(ssid.c_str(), key.c_str());
  digitalWrite(red, HIGH); // Turn on the red LED while connecting
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  // Once connected red LED off and green LED on
  digitalWrite(red, LOW);
  digitalWrite(green, HIGH); // Turn on the green LED once connected
  Serial.println("Connected to Wi-Fi.");
  delay(10000);
  digitalWrite(green, LOW); //Turn off the Green LED after 10s
  // Send responce to the HTTP GET request showing connection was a success
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  server.send(200, "text/html", "<html><body><h1>Connected to " + ssid + "</h1><p><a href='/'>Go back to Home Page</a></p></body></html>");
}
// 404 Error for pages that dont exist
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

void setup() {
  // Wait for 15 seconds before executing the rest of the code to load up serial monitor
  delay(15000);
 // Set up the pins and Button
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  // Turn on red LED when it starts
  digitalWrite(red, HIGH); 
  // Initialize the serial port and start access point
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  // Set up web server and defind endpoints
  server.on("/", handleRoot);
  server.on("/connect", handleConnect);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Web server started");
}

// Handle incoming HTTP requests and check if the button has been pressed
void loop() {
  server.handleClient();
  // If button pressed perform OTA update
  if (digitalRead(buttonPin) == LOW) {
    performOTAUpdate();
    delay(300); // Add a small delay to debounce the button
  }
}

void performOTAUpdate() {
  // Yellow LED on to indicate OTA update
  digitalWrite(yellow, HIGH);
  // Connect to server and check for a new firmware 
  HTTPClient http;
  char url[256];

  snprintf(url, sizeof(url), "http://%s:%d/version.txt", FIRMWARE_SERVER_IP_ADDR, FIRMWARE_SERVER_PORT);
  http.begin(url);
  int httpCode = http.GET();
  // If new version available download and install
  if (httpCode > 0) {
    int latestVersion = atoi(http.getString().c_str());
    if (latestVersion > firmwareVersion) {
      snprintf(url, sizeof(url), "http://%s:%d/firmware_%d.bin", FIRMWARE_SERVER_IP_ADDR, FIRMWARE_SERVER_PORT, latestVersion);
      http.begin(url);
      int httpCode = http.GET();
      // If successful install the new firmware and reboot
      if (httpCode == 200) {
        Serial.println("Starting OTA update...");
        // Check if there is enough space
        if (Update.begin(http.getSize())) {
          // Write the new firmware
          size_t written = Update.writeStream(*http.getStreamPtr());
          if (written == http.getSize()) {
            Serial.println("Written : " + String(written) + " successfully");
          } else {
            Serial.println("Written only : " + String(written) + "/" + String(http.getSize()) + ". Retry?" );
          }
          if (Update.end()) {
            Serial.println("OTA update complete. Rebooting...");
            digitalWrite(green, HIGH); // Turn on the green LED
            digitalWrite(yellow, LOW); // Turn off the yellow LED
            delay(10000); // Wait for 10 seconds
            digitalWrite(green, LOW); // Turn off the green LED
            ESP.restart(); // Restart the ESP
          } else {
            // Case for error in the OTA update
            Serial.println("Error Occurred. Error #: " + String(Update.getError()));
            digitalWrite(red, HIGH); // Turn on the red LED
            digitalWrite(yellow, LOW); // Turn off the yellow LED
            delay(10000); // Wait for 10 seconds
            digitalWrite(red, LOW); // Turn off the red LED
          }
        } else {
          // Case where there isnt enough space
          Serial.println("Not enough space to begin OTA");
          digitalWrite(red, HIGH); // Turn on the red LED
          digitalWrite(yellow, LOW); // Turn off the yellow LED
          delay(10000); // Wait for 10 seconds
          digitalWrite(red, LOW); // Turn off the red LED
        }
      } else {
        // Case where firmware fails to download
        Serial.printf("Unable to fetch firmware: HTTP code %d\n", httpCode);
        digitalWrite(red, HIGH); // Turn on the red LED
        digitalWrite(yellow, LOW); // Turn off the yellow LED
        delay(10000); // Wait for 10 seconds
        digitalWrite(red, LOW); // Turn off the red LED
      }
    } else {
      // Case of firmware up to date
      Serial.println("Firmware is up to date");
      digitalWrite(red, HIGH); // Turn on the red LED
      digitalWrite(yellow, LOW); // Turn off the yellow LED
      delay(10000); // Wait for 10 seconds
      digitalWrite(red, LOW); // Turn off the red LED
    }
  } else {
    // Case where version.txt could not be found
    Serial.printf("Unable to fetch version.txt: HTTP code %d\n", httpCode);
    digitalWrite(red, HIGH); // Turn on the red LED
    digitalWrite(yellow, LOW); // Turn off the yellow LED
    delay(10000); // Wait for 10 seconds
    digitalWrite(red, LOW); // Turn off the red LED
  }
  // End the session for HTTP
  http.end();
}
