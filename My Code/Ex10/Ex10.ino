#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>

const char* ssid = "FLAT-6-2G";
const char* password = "ptpassword";

const char* server_ip = "192.168.1.100"; // IP address of your local HTTP server
const int server_port = 8000; // Port of your local HTTP server
const String firmware_url = "/firmware.bin"; // URL of the firmware binary file

int firmwareVersion = 1; // current firmware version

WebServer server(80);

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/update", HTTP_POST, [](){
    server.send(200, "text/plain", "OTA update started.");
    delay(1000);

    WiFiClient client;
    if (client.connect(server_ip, server_port)) {
      Serial.println("Connected to server, downloading firmware...");
      client.print(String("GET ") + firmware_url + " HTTP/1.1\r\n" +
                   "Host: " + server_ip + "\r\n" +
                   "Connection: close\r\n\r\n");

      while(client.connected()){
        if(client.available()){
          String line = client.readStringUntil('\n');
          if(line == "\r"){
            Serial.println("Headers received, downloading firmware binary...");
            break;
          }
        }
      }

      if (Update.begin()) {
        while(client.connected()){
          uint8_t buff[128] = { 0 };
          int len = client.readBytes(buff, 128);
          if(len > 0){
            Update.write(buff, len);
          }
          else{
            break;
          }
        }

        if (Update.end()) {
          Serial.println("Firmware updated successfully.");
          firmwareVersion++; // increment firmware version
          // update firmwareVersion in the .ino file
          String ino_file_content = "";
          File ino_file = SPIFFS.open("/main.ino", "r");
          while (ino_file.available()) {
            ino_file_content += (char)ino_file.read();
          }
          ino_file.close();
          ino_file_content.replace("int firmwareVersion = " + String(firmwareVersion - 1), "int firmwareVersion = " + String(firmwareVersion));
          ino_file = SPIFFS.open("/main.ino", "w");
          ino_file.print(ino_file_content);
          ino_file.close();
        }
        else {
          Serial.println("Error updating firmware.");
        }
      }
      else {
        Serial.println("Could not begin OTA update.");
      }
    }
    else {
      Serial.println("Could not connect to server.");
    }
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
