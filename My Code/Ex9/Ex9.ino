#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32 Access Point"; // SSID of the ESP32 access point
const char* password = "password"; // Password for the access point

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
  Serial.println("Connected to Wi-Fi.");
  
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
  // Check if the button is pressed
  if (digitalRead(buttonPin) == LOW) {
    // Mock update process
    mockUpdate();
    delay(300); // Add a small delay to debounce the button
  }
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
