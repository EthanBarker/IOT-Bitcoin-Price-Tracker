#include <WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "FLAT-6-5G";
const char* password = "ptpassword";
const char* host = "ec2-52-51-219-177.eu-west-1.compute.amazonaws.com";
const int httpsPort = 443;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  sendEmailAndMAC();
}

void sendEmailAndMAC() {
  // Get MAC address of ESP32
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String macAddress = "";
  for (int i = 0; i < 6; i++) {
    macAddress += String(mac[i], HEX);
    if (i < 5) {
      macAddress += ":";
    }
  }
  Serial.print("MAC address: ");
  Serial.println(macAddress);

  // Create WiFiClientSecure object
  WiFiClientSecure client;

  // Connect to server
  client.setInsecure(); // allow self-signed certificate
  if (!client.connect(host, httpsPort)) {
    Serial.println("Error connecting to server");
    return;
  }

  // Construct POST data
  String postData = "email=ebarker4@sheffield.ac.uk&mac=" + macAddress;

  // Send POST request
  String request = "POST / HTTP/1.1\r\n" +
                   String("Host: ") + host + "\r\n" +
                   "Content-Type: application/x-www-form-urlencoded\r\n" +
                   "Content-Length: " + String(postData.length()) + "\r\n\r\n" +
                   postData;
  client.print(request);

  // Check for response
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  String response = client.readString();
  Serial.println(response);

  // Cleanup
  client.stop();
}

void loop() {
  // Do nothing
}
