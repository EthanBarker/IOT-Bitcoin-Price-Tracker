#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Ethan's very cool ESP32"; // Replace with your SSID
const char* password = "Password123"; // Replace with your password

WebServer webServer(80); // Create a web server on port 80

void handleRoot() {
  String page = "<html><body><h1>Available Wi-Fi Networks</h1><ul>";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    page += "<li>";
    page += WiFi.SSID(i);
    page += "  <a href=\"/connect?ssid=";
    page += WiFi.SSID(i);
    page += "\">Connect</a>";
    page += "</li>";
  }
  page += "</ul></body></html>";
  webServer.send(200, "text/html", page);
}

void handleConnect() {
  String ssid = webServer.arg("ssid");
  String key = webServer.arg("key");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid.c_str(), key.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  webServer.send(200, "text/html", "<html><body><h1>Connected to " + ssid + "</h1><p><a href='/'>Go back to Home Page</a></p></body></html>");
  WiFi.printDiag(Serial);
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
  webServer.on("/connect", handleConnect);
  webServer.onNotFound(handleNotFound);

  // Start the server
  webServer.begin();

  Serial.println("Web server started");
}

void loop() {
  // Handle incoming client requests
  webServer.handleClient();
}
