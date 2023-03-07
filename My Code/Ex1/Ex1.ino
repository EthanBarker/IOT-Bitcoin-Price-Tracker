char mac_address[18]; // MAC addresses are 6 bytes, which translates to 12 hexadecimal digits, plus the NULL terminator

void setup() {
  Serial.begin(115200); // initialize the serial line
  uint8_t mac[6]; // array to store MAC address bytes
  esp_efuse_mac_get_default(mac); // get the default MAC address
  sprintf(mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); // format the MAC address as a string
  pinMode(LED_BUILTIN, OUTPUT); // set up the built-in LED pin as an output
}

void loop() {
  Serial.println(mac_address); // print the MAC address on the serial line
  digitalWrite(LED_BUILTIN, HIGH); // turn on the built-in LED
  delay(100); // wait for 100ms
  digitalWrite(LED_BUILTIN, LOW); // turn off the built-in LED
  delay(4900); // wait for 4900ms (5 seconds minus the 100ms delay above)
}
