#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Ethan's very cool ESP32"; // Replace with your SSID
const char* password = "Password123"; // Replace with your password

WebServer webServer(80); // Create a web server on port 80

void handleRoot() {
  webServer.send(200, "text/html", "<html><body><h1>Welcome to my website, this is the homepage</h1><p><a href='/page1'>Go to Page 1</a></p><p><a href='/page2'>Go to Page 2</a></p></body></html>");
}

void handlePage1() {
  webServer.send(200, "text/html", "<html><body><h1>This is Page 1</h1><p><a href='/'>Go back to Home Page</a></p></body></html>");
}

void handlePage2() {
  webServer.send(200, "text/html", "<html><body><h1>This is Page 2</h1><p><a href='/'>Go back to Home Page</a></p></body></html>");
}

void handleNotFound() {
  webServer.send(404, "text/plain", "404: Not found");
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

  // Register web page handlers
  webServer.on("/", handleRoot);
  webServer.on("/page1", handlePage1);
  webServer.on("/page2", handlePage2);
  webServer.onNotFound(handleNotFound);

  // Start the server
  webServer.begin();

  Serial.println("Web server started");
}

void loop() {
  // Handle incoming client requests
  webServer.handleClient();
}
