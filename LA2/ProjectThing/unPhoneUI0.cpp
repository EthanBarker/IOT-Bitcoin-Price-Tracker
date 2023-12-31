// unPhoneUI0.cpp //////////////////////////////////////////////////////////

#if UNPHONE_UI0 == 1
#include "unPhoneUI0.h"
#include <WiFi.h>                 // status for config scrn
#include <Adafruit_ImageReader.h> // image-reading for test card scrn
#include <HTTPClient.h>
#include <ArduinoJson.h>

static unPhone *up;

// initialisation flag, not complete until parent has finished config
bool UIController::provisioned = false;

// Bitcoin referesh buttons
int refreshRateIndex = 1; // Start with the 10-second refresh rate
const long refreshRates[] = {1000, 10000, 30000, 60000, 3600000, 86400000};
const int buttonPositions[][2] = {{10, 120}, {165, 120}, {10, 180}, {165, 180}, {10, 240}, {165, 240}};
const char* buttonTexts[] = {"1s", "10s", "30s", "1m", "1h", "24h"};

// the UI elements types (screens) /////////////////////////////////////////
const char *UIController::ui_mode_names[] = {
  "Menu",
  "Home",
  "Wifi",
  "Bitcoin",
  "Heads or Tails",
};
uint8_t UIController::NUM_UI_ELEMENTS = 5;  // number of UI elements

// keep Arduino IDE compiler happy /////////////////////////////////////////
UIElement::UIElement(Adafruit_HX8357* tftp, XPT2046_Touchscreen* tsp, SdFat *sdp) {
  m_tft = tftp;
  m_ts = tsp;
  m_sd = sdp;
}
void UIElement::someFuncDummy() { }

// constructor for the main class ///////////////////////////////////////////
UIController::UIController(ui_modes_t start_mode) {
  m_mode = start_mode;
}

bool UIController::begin(unPhone& u) { ///////////////////////////////////////
  up = &u;
  begin(u, true);
  return true;
}
bool UIController::begin(unPhone& u, boolean doDraw) { ///////////////////////
  up = &u;
  D("UI.begin()\n")

  up->tftp->fillScreen(HX8357_GREEN);
  WAIT_MS(50)
  up->tftp->fillScreen(HX8357_BLACK);
  
  // define the menu element and the first m_element here 
  m_menu = new MenuUIElement(up->tftp, up->tsp, up->sdp);
  if(m_menu == NULL) {
    Serial.println("ERROR: no m_menu allocated");
    return false;
  }
  allocateUIElement(m_mode);

  if(doDraw)
    redraw();
  return true;
}

UIElement* UIController::allocateUIElement(ui_modes_t newMode) {
  // TODO trying to save memory here, but at the expense of possible
  // fragmentation; perhaps maintain an array of elements and never delete?
  if(m_element != 0 && m_element != m_menu) delete(m_element);

  switch(newMode) {
    case ui_menu:
      m_element = m_menu;                                               break;
    case ui_configure:
      m_element = new ConfigUIElement(up->tftp, up->tsp, up->sdp);      break;
    case ui_wifi: 
      m_element = new WifiUIElement(up->tftp, up->tsp, up->sdp);        break; 
    case ui_bitcoin_price:
      m_element = new BitcoinPriceUIElement(up->tftp, up->tsp, up->sdp, *up); break;
    case ui_headsortails:
      m_element = new HeadsOrTailsUIElement(up->tftp, up->tsp, up->sdp, *up);  break;           
    default:
      Serial.printf("invalid UI mode %d in allocateUIElement\n", newMode);
      m_element = m_menu;
  }

  return m_element;
}

// touch management code ////////////////////////////////////////////////////
TS_Point nowhere(-1, -1, -1);    // undefined coordinate
TS_Point firstTouch(0, 0, 0);    // the first touch defaults to 0,0,0
TS_Point p(-1, -1, -1);          // current point of interest (signal)
TS_Point prevSig(-1, -1, -1);    // the previous accepted touch signal
bool firstTimeThrough = true;    // first time through gotTouch() flag
uint16_t fromPrevSig = 0;        // distance from previous signal
unsigned long now = 0;           // millis
unsigned long prevSigMillis = 0; // previous signal acceptance time
unsigned long sincePrevSig = 0;  // time since previous signal acceptance
uint16_t DEFAULT_TIME_SENSITIVITY = 150; // min millis between touches
uint16_t TIME_SENSITIVITY = DEFAULT_TIME_SENSITIVITY;
uint16_t DEFAULT_DIST_SENSITIVITY = 200; // min distance between touches
uint16_t DIST_SENSITIVITY = DEFAULT_DIST_SENSITIVITY;
uint16_t TREAT_AS_NEW = 600;     // if no signal in this period treat as new
uint8_t MODE_CHANGE_TOUCHES = 1; // number of requests needed to switch mode
uint8_t modeChangeRequests = 0;  // number of current requests to switch mode
bool touchDBG = false;           // set true for diagnostics

void setTimeSensitivity(uint16_t s = DEFAULT_TIME_SENSITIVITY) { ////////////
  TIME_SENSITIVITY = s;
}
void setDistSensitivity(uint16_t d = DEFAULT_DIST_SENSITIVITY) { ////////////
  DIST_SENSITIVITY = d;
}
uint16_t distanceBetween(TS_Point a, TS_Point b) { // coord distance ////////
  uint32_t xpart = b.x - a.x, ypart = b.y - a.y;
  xpart *= xpart; ypart *= ypart;
  return sqrt(xpart + ypart);
}
void dbgTouch() { // print current state of touch model /////////////////////
  if(touchDBG) {
    D("p(x:%04d,y:%04d,z:%03d)", p.x, p.y, p.z)
    D(", now=%05lu, sincePrevSig=%05lu, prevSig=", now, sincePrevSig)
    D("p(x:%04d,y:%04d,z:%03d)", prevSig.x, prevSig.y, prevSig.z)
    D(", prevSigMillis=%05lu, fromPrevSig=%05u", prevSigMillis, fromPrevSig)
  }
}
const char *UIController::modeName(ui_modes_t m) {
  switch(m) {
    case ui_menu:               return "ui_menu";          break;
    case ui_configure:          return "ui_configure";     break;
    case ui_wifi:               return "ui_wifi";          break;
    case ui_bitcoin_price:      return "Bitcoin Price";    break;
    case ui_headsortails:       return "ui_headsortails";  break;    
    default:
      D("invalid UI mode %d in allocateUIElement\n", m)
      return "invalid UI mode";
  }
}

// accept or reject touch signals ///////////////////////////////////////////
bool UIController::gotTouch() { 
  if(!up->tsp->touched()) {
    return false; // no touches
  }
    
  // set up timings
  now = millis();
  if(firstTimeThrough) {
    sincePrevSig = TIME_SENSITIVITY + 1;
  } else {
    sincePrevSig = now - prevSigMillis;
  }

  // retrieve a point
  p = up->tsp->getPoint();
  // add the following if want to dump the rest of the buffer:
  // while (! up->tsp->bufferEmpty()) {
  //   uint16_t x, y;
  //   uint8_t z;
  //   up->tsp->readData(&x, &y, &z);
  // }
  // delay(300);
  if(touchDBG)
    D("\n\np(x:%04d,y:%04d,z:%03d)\n\n", p.x, p.y, p.z)

  // if it is at 0,0,0 and we've just started then ignore it
  if(p == firstTouch && firstTimeThrough) {
    dbgTouch();
    if(touchDBG) D(", rejecting (0)\n\n")
    return false;
  }
  firstTimeThrough = false;
  
  // calculate distance from previous signal
  fromPrevSig = distanceBetween(p, prevSig);
  dbgTouch();

  if(touchDBG)
    D(", sincePrevSig<TIME_SENS.: %d...  ", sincePrevSig<TIME_SENSITIVITY)
  if(sincePrevSig < TIME_SENSITIVITY) { // ignore touches too recent
    if(touchDBG) D("rejecting (2)\n")
  } else if(
    fromPrevSig < DIST_SENSITIVITY && sincePrevSig < TREAT_AS_NEW
  ) {
    if(touchDBG) D("rejecting (3)\n")
#if UNPHONE_SPIN >= 9
  } else if(p.z < 400) { // ghost touches in 9 (on USB power) are ~300 pressure
    // or ignore: x > 1200 && x < 1700 && y > 2000 && y < 3000 && z < 450 ?
    if(touchDBG) D("rejecting (4)\n") // e.g. p(x:1703,y:2411,z:320)
#endif
  } else {
    prevSig = p;
    prevSigMillis = now;
    if(false) // delete this line to debug touch debounce
      D("decided this is a new touch: p(x:%04d,y:%04d,z:%03d)\n", p.x, p.y, p.z)
    return true;
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////
void UIController::changeMode() {
  D("changing mode from %d (%s) to...", m_mode, modeName(m_mode))
  up->tftp->fillScreen(HX8357_BLACK);
  setTimeSensitivity();         // set TIME_SENS to the default
  nextMode = (ui_modes_t) ((MenuUIElement *)m_menu)->getMenuItemSelected();
  if(nextMode == -1) nextMode = ui_menu;

  // allocate an element according to nextMode and 
  if(m_mode == ui_menu) {       // coming OUT of menu
    m_mode =    nextMode;
    m_element = allocateUIElement(nextMode);
  } else {                      // going INTO menu
    m_mode =    ui_menu;
    m_element = m_menu;
  }
  D("...%d (%s)\n", m_mode, modeName(m_mode))

  redraw();
  return;
}

/////////////////////////////////////////////////////////////////////////////
void UIController::handleTouch() {
  int temp = p.x;
  p.x = map(p.y, up->TS_MAXX, up->TS_MINX, 0, up->tftp->width());
  p.y = map(temp, up->TS_MAXY, up->TS_MINY, 0, up->tftp->height());
  // Serial.print("dbgTouch from handleTouch: "); dbgTouch(); Serial.flush();
  
  if(m_element->handleTouch(p.x, p.y)) {
    if(++modeChangeRequests >= MODE_CHANGE_TOUCHES) {
      changeMode();
      modeChangeRequests = 0;
    }
  } 
}

/////////////////////////////////////////////////////////////////////////////
void UIController::run() {
  if(gotTouch())
    handleTouch();
  m_element->runEachTurn();
}

////////////////////////////////////////////////////////////////////////////
void UIController::redraw() {
  up->tftp->fillScreen(HX8357_BLACK);
  m_element->draw();
}

////////////////////////////////////////////////////////////////////////////
void UIController::message(char *s) {
  up->tftp->setCursor(0, 465);
  up->tftp->setTextSize(2);
  up->tftp->setTextColor(HX8357_CYAN, HX8357_BLACK);
  up->tftp->print("                          ");
  up->tftp->setCursor(0, 465);
  up->tftp->print(s);
}

////////////////////////////////////////////////////////////////////////////
void UIElement::drawSwitcher(uint16_t xOrigin, uint16_t yOrigin) {
  uint16_t leftX = xOrigin;
  if(leftX == 0)
    leftX = (SWITCHER * BOXSIZE) + 8; // default is on right hand side
    m_tft->fillRect(leftX, 15 + yOrigin, BOXSIZE - 15, HALFBOX - 10, WHITE);
    m_tft->fillTriangle(
      leftX + 15, 35 + yOrigin,
      leftX + 15,  5 + yOrigin,
      leftX + 30, 20 + yOrigin,
      WHITE
    );
}

////////////////////////////////////////////////////////////////////////////
void UIElement::showLine(const char *buf, uint16_t *yCursor) {
  *yCursor += 20;
  m_tft->setCursor(0, *yCursor);
  m_tft->print(buf);
}


//////////////////////////////////////////////////////////////////////////////
// ConfigUIElement.cpp ///////////////////////////////////////////////////////

/**
 * Handle touches on this page
 * 
 * @param x - the x coordinate of the touch 
 * @param y - the y coordinate of the touch 
 * @returns bool - true if the touch is on the switcher
 */
bool ConfigUIElement::handleTouch(long x, long y) {
  return y < BOXSIZE && x > (BOXSIZE * SWITCHER);
}

// writes various things including mac address and wifi ssid ///////////////
void ConfigUIElement::draw() {
  // say hello
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(2);
  uint16_t yCursor = 0;
  m_tft->setCursor(0, yCursor);
  m_tft->print("Welcome to unPhone!");
  m_tft->setTextColor(BLUE);

  // note about switcher
  yCursor += 20;
  if(UIController::provisioned) {
    showLine("(where you see the arrow,", &yCursor);
    showLine("  press for menu)", &yCursor);
    drawSwitcher();
  } else {
    yCursor += 20;
  }

  // are we connected?
  yCursor += 40;
  m_tft->setCursor(0, yCursor);
  if (WiFi.status() == WL_CONNECTED) {
    yCursor += 20;
    m_tft->print("Connected to: ");
    m_tft->setTextColor(GREEN);
    m_tft->print(WiFi.SSID());
    m_tft->setTextColor(BLUE);
  } else {
    m_tft->setTextColor(RED);
    m_tft->print("Not connected to WiFi:");
    yCursor += 20;
    m_tft->setCursor(0, yCursor);
    m_tft->print("  trying to connect...");
    m_tft->setTextColor(BLUE);
  }

  // display the mac address
  char mac_buf[13];
  yCursor += 40;
  m_tft->setCursor(0, yCursor);
  m_tft->print("MAC addr: ");
  m_tft->print(up->getMAC());

  // firmware version
  showLine("Firmware date:", &yCursor);
  showLine("  ", &yCursor);
  m_tft->print(up->buildTime);

  // (used to be) AP details, now just unPhone.name
  showLine("Firmware name: ", &yCursor);
  showLine("  ", &yCursor);
  m_tft->print(up->appName);
  //m_tft->print("-");
  //m_tft->print(up->getMAC());

  // IP address
  showLine("IP: ", &yCursor);
  m_tft->print(WiFi.localIP());

  // battery voltage
  showLine("VBAT: ", &yCursor);
  m_tft->print(up->batteryVoltage());

  // battery voltage
  showLine("Hardware version: ", &yCursor);
  m_tft->print(UNPHONE_SPIN);

  // display the on-board temperature
  char buf[256];
  float onBoardTemp = temperatureRead();
  sprintf(buf, "MCU temp: %.2f C", onBoardTemp);
  showLine(buf, &yCursor);

  // web link
  yCursor += 60;
  showLine("An ", &yCursor);
  m_tft->setTextColor(MAGENTA);
  m_tft->print("IoT platform");
  m_tft->setTextColor(BLUE);
  m_tft->print(" from the");
  m_tft->setTextColor(MAGENTA);
  showLine("  University of Sheffield", &yCursor);
  m_tft->setTextColor(BLUE);
  showLine("Find out more at", &yCursor);
  m_tft->setTextColor(GREEN);
  showLine("              unphone.net", &yCursor);
}

//////////////////////////////////////////////////////////////////////////////
void ConfigUIElement::runEachTurn() {
  
}


//////////////////////////////////////////////////////////////////////////////
// MenuUIElement.cpp /////////////////////////////////////////////////////////

/**
 * Process touches.
 * @returns bool - true if the touch is a menu item
 */
bool MenuUIElement::handleTouch(long x, long y) {
  // D("text mode: responding to touch @ %d/%d/%d: ", x, y,-1)
  m_tft->setTextColor(WHITE, BLACK);
  int8_t menuItem = mapTextTouch(x, y);
  D("menuItem=%d, ", menuItem)
  if(menuItem == -1) D("ignoring\n")

  if(menuItem > 0 && menuItem < UIController::NUM_UI_ELEMENTS) {
    menuItemSelected = menuItem;
    return true;
  }
  return false;
}

// returns menu item number //////////////////////////////////////////////////
int8_t MenuUIElement::mapTextTouch(long xInput, long yInput) {
  for(
    int y = 30, i = 1;
    i < UIController::NUM_UI_ELEMENTS && y < 480;
    y += 48, i++
  ) {
    if(xInput > 270 && yInput > y && yInput < y + 48)
      return i;
  }
  return -1;
}

// draw a textual menu ///////////////////////////////////////////////////////
void MenuUIElement::draw(){
  m_tft->setTextSize(2);
  m_tft->setTextColor(BLUE);

  m_tft->setCursor(230, 0);
  m_tft->print("MENU");

  uint16_t yCursor = 30;
  m_tft->drawFastHLine(0, yCursor, 320, MAGENTA);
  yCursor += 16;

  for(int i = 1; i < UIController::NUM_UI_ELEMENTS; i++) {
    m_tft->setCursor(0, yCursor);
    m_tft->print(UIController::ui_mode_names[i]);
    drawSwitcher(288, yCursor - 12);
    yCursor += 32;
    m_tft->drawFastHLine(0, yCursor, 320, MAGENTA);
    yCursor += 16;
  }
}

//////////////////////////////////////////////////////////////////////////////
void MenuUIElement::runEachTurn(){ // text page UI, run each turn
  // do nothing
}
//////////////////////////////////////////////////////////////////////////////
void WifiUIElement::draw() {
  m_tft->fillScreen(BLACK);
  scanNetworks();
  displayNetworks();

  // Draw the back button
  m_tft->fillRoundRect(0, 430, 320, 50, 5, WHITE);
  m_tft->setTextColor(BLACK);
  m_tft->setTextSize(2);
  m_tft->setCursor(10, 440);
  m_tft->print("Back to Menu ");
  m_tft->print((char)24); // Arrow pointing left
}

bool WifiUIElement::handleTouch(long x, long y) {
  // Check if the back button was touched
  if (y >= 430 && y <= 480) {
    return true; // Go back to the main menu
  }

  // Check if a network was selected
  int startY = 50; // Starting y-coordinate for network list
  int lineHeight = 30; // Height for each line (adjust as needed)

  for (int i = 0; i < numberOfNetworks; ++i) {
    int yTop = startY + i * lineHeight; // Compute y-coordinates for each line
    int yBottom = yTop + lineHeight;

    if (x >= 10 && x <= 310 && y >= yTop && y <= yBottom) {
      // A network was selected, connect to it here
      connectToNetwork(networkNames[i]);
      break;
    }
  }

  return false; // Stay on the current screen
}

void WifiUIElement::connectToNetwork(String networkName) {
  // Assuming you know the password or it's an open network
  String password = "ptpassword";

  WiFi.begin(networkName.c_str(), password.c_str());

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void WifiUIElement::runEachTurn() {
  // No code to run each turn
}

void WifiUIElement::initWiFi() {
  // Set WiFi mode to station
  WiFi.mode(WIFI_STA);

  // Disconnect from any previous network
  WiFi.disconnect();
  
  // Wait for the WiFi module to be ready
  delay(100);
}

void WifiUIElement::scanNetworks() {
  // Initialize WiFi
  initWiFi();

  Serial.print("Number of networks found: ");
  Serial.println(numberOfNetworks);
  numberOfNetworks = WiFi.scanNetworks();
  networkNames.clear();
  
  if (numberOfNetworks == -1) {
    Serial.println("Error scanning for networks. Please check the WiFi connection.");
    return;
  }
  
  for (int i = 0; i < numberOfNetworks; ++i) {
    Serial.print("Network ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(WiFi.SSID(i));
    networkNames.push_back(WiFi.SSID(i));
  }
}

void WifiUIElement::displayNetworks() {
  m_tft->setTextColor(WHITE);
  m_tft->setTextSize(3); // Increase text size

  int startY = 50; // Starting y-coordinate for network list
  int lineHeight = 30; // Height for each line (adjust as needed)

  for (int i = 0; i < numberOfNetworks; ++i) {
    int yPos = startY + i * lineHeight; // Compute y-coordinate for each line
    m_tft->setCursor(10, yPos);
    m_tft->println(networkNames[i]);

    if (i < numberOfNetworks - 1) { // Don't draw a line after the last network
      m_tft->drawLine(10, yPos + lineHeight - 5, 310, yPos + lineHeight - 5, WHITE); // Draw a line
    }
  }
}
//////////////////////////////////////////////////////////////////////////////
void BitcoinPriceUIElement::updateBitcoinPrice() {
  // Store the previous price before updating
  previous_price = bitcoin_price.toFloat();  
  HTTPClient http;
  http.begin("https://api.coinbase.com/v2/prices/spot?currency=USD");
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    bitcoin_price = doc["data"]["amount"].as<String>();
  } else {
    Serial.print("Error fetching Bitcoin price: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}

void BitcoinPriceUIElement::draw() {
  m_tft->fillScreen(BLACK);
  m_tft->setTextColor(WHITE);
  m_tft->setTextSize(3);
  m_tft->setCursor(10, 10);
  m_tft->print("Bitcoin Price:");

  // Draw the back button
  m_tft->fillRoundRect(0, 430, 320, 50, 5, WHITE);
  m_tft->setTextColor(BLACK);
  m_tft->setTextSize(2);
  m_tft->setCursor(10, 440);
  m_tft->print("Back to Menu ");
  m_tft->print((char)24); // Arrow pointing left

  // Draw the buttons
  m_tft->setTextSize(2);
  for (int i = 0; i < 6; i++) {
    m_tft->fillRoundRect(buttonPositions[i][0], buttonPositions[i][1], 145, 50, 5, WHITE);
    m_tft->setTextColor(BLACK);
    m_tft->setCursor(buttonPositions[i][0] + 45, buttonPositions[i][1] + 15);
    m_tft->print(buttonTexts[i]);
    m_tft->setTextColor(WHITE);
  }

  updateAndDrawPrice();
}

bool BitcoinPriceUIElement::handleTouch(long x, long y) {
  // Check if the back button was touched
  if (y >= 420 && y <= 460) {
    return true; // Go back to the main menu
  }

  // Check if any refresh rate buttons were touched
  for (int i = 0; i < 6; i++) {
    if (x >= buttonPositions[i][0] && x <= buttonPositions[i][0] + 90 && y >= buttonPositions[i][1] && y <= buttonPositions[i][1] + 40) {
      refreshRateIndex = i;
      showRefreshRateChangeMessage(); // Call the function when a button is pressed
      
      buzzTimes(i + 1);
      break;
    }
  }

  return false; // Stay on the current screen
}

void BitcoinPriceUIElement::updateAndDrawPrice() {
  updateBitcoinPrice();

  // Clear the previous price and arrow
  m_tft->fillRect(10, 60, 300, 40, BLACK);

  // Buffer to store the formatted price string
  char formatted_price[10];

  // Format the price with two decimal places
  sprintf(formatted_price, "%.2f", bitcoin_price.toFloat());

  if (bitcoin_price.toFloat() > previous_price) {
    m_tft->setTextColor(GREEN);
  } else if (bitcoin_price.toFloat() < previous_price) {
    m_tft->setTextColor(RED);
  } else {
    m_tft->setTextColor(WHITE);
  }

  m_tft->setTextSize(4);
  m_tft->setCursor(10, 60);
  m_tft->print("$");
  m_tft->print(formatted_price); // Print the formatted price

  // Draw the arrow
  m_tft->setTextSize(4);
  m_tft->setCursor(250, 60);
  if (bitcoin_price.toFloat() > previous_price) {
    m_tft->print((char)24); // Up arrow
  } else if (bitcoin_price.toFloat() < previous_price) {
    m_tft->print((char)25); // Down arrow
  } else {
    m_tft->print("-"); // No change
  }
}

unsigned long previousMillis = 0;
const long interval = 10000; 

void BitcoinPriceUIElement::runEachTurn() {
  unsigned long currentMillis = millis();

  // Check if it's time to clear the refresh rate change message
  if (messageClearTime > 0 && currentMillis - messageClearTime >= 5000) {
    m_tft->fillRect(0, 375, 320, 20, BLACK); // Clear the message area
    messageClearTime = 0; // Reset the message clear time
  }

  if (currentMillis - previousMillis >= refreshRates[refreshRateIndex]) {
    // Save the last time the Bitcoin price was updated
    previousMillis = currentMillis;

    // Update and redraw the Bitcoin price
    updateAndDrawPrice(); 
  }
}

void BitcoinPriceUIElement::showRefreshRateChangeMessage() {
  // Clear the previous message, if any
  m_tft->fillRect(0, 375, 320, 20, BLACK);

  // Show the new message
  m_tft->setTextSize(2);
  m_tft->setTextColor(WHITE);
  m_tft->setCursor(10, 375);
  m_tft->print("Refresh rate changed to ");
  m_tft->print(buttonTexts[refreshRateIndex]);

  // Set the time when the message should be cleared
  messageClearTime = millis();
}
//////////////////////////////////////////////////////////////////////////////
void HeadsOrTailsUIElement::draw() {
  // Clear the screen
  m_tft->fillScreen(BLACK);

  // Draw the title
  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(3);
  m_tft->setCursor(40, 10);
  m_tft->println("Heads or Tails");

  // Draw the current score
  m_tft->setTextColor(YELLOW);
  m_tft->setTextSize(2);
  m_tft->setCursor(10, 50);
  m_tft->print("Score: ");
  m_tft->println(score);

  // Draw the best score
  m_tft->setTextColor(YELLOW);
  m_tft->setCursor(10, 80);
  m_tft->print("Best: ");
  m_tft->println(bestScore);

  // Draw the remaining lives
  m_tft->setTextColor(YELLOW);
  m_tft->setTextSize(2);
  m_tft->setCursor(10, 110);
  m_tft->print("Lives: ");
  m_tft->println(lives);

  // Draw the buttons for "Heads" and "Tails"
  m_tft->fillRoundRect(30, 200, 120, 80, 5, GREEN); // "Heads" button
  m_tft->fillRoundRect(170, 200, 120, 80, 5, RED); // "Tails" button

  // Draw the back button
  m_tft->fillRoundRect(100, 350, 120, 60, 5, BLUE);
  m_tft->setTextColor(WHITE);
  m_tft->setTextSize(2);
  m_tft->setCursor(120, 370);
  m_tft->print("Back");

  // Label the buttons
  m_tft->setTextColor(WHITE);
  m_tft->setCursor(60, 240);
  m_tft->print("Heads");
  m_tft->setCursor(200, 240);
  m_tft->print("Tails");
}

bool HeadsOrTailsUIElement::handleTouch(long x, long y) {
  // Check if the back button was touched
  if (x >= 100 && x <= 220 && y >= 350 && y <= 410) {
    return true; // Go back to the main menu
  }  
  if (x >= 30 && x <= 150 && y >= 200 && y <= 280) {
    playGame(true); // The "Heads" button was pressed
  } else if (x >= 170 && x <= 290 && y >= 200 && y <= 280) {
    playGame(false); // The "Tails" button was pressed
  }

  return false; // Stay on the current screen
} 

void HeadsOrTailsUIElement::runEachTurn() {
  // No code to run each turn
}


void HeadsOrTailsUIElement::playGame(bool heads) {
  // Flip a coin (true = heads, false = tails)
  bool coin = random(2); 

  if (coin == heads) {
    Serial.println("You win!");
    ++score; // Increase the score

    // Check if this is a new high score
    if (score > bestScore) {
      bestScore = score;
    }
  } else {
    Serial.println("You lose!");
    --lives; // Decrement the lives
    buzz(); // Buzz to signal a loss

    if (lives == 0) {
      score = 0; // Reset the score
      lives = 3; // Reset the lives
    }
  }

  // Redraw the screen to update the score and lives
  draw();
}
//////////////////////////////////////////////////////////////////////////////
#endif // PREDICTOR_MAIN
