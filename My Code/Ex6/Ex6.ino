#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Ethan's very cool ESP32"; // Replace with your SSID
const char* password = "Password123"; // Replace with your password

WebServer webServer(80); // Create a web server on port 80

void handleRoot() {
  webServer.send(200, "text/html", "<html><body><h1>Alice is a cool bean!</h1></body></html>");
}

void setup() {
  Serial.begin(115200); // Start serial communication

  // Connect to WiFi access point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("WiFi access point started");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);

  // Register web page handler
  webServer.on("/", handleRoot);

  // Start the server
  webServer.begin();

  Serial.println("Web server started");
}

void loop() {
  // Handle incoming client requests
  webServer.handleClient();
}
