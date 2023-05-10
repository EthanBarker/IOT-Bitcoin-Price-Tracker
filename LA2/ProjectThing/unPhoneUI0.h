// unPhoneUI0.h
// agglomerated default UI mostly derived from HarryEH's code
// and Mark Hepple's predictive text python code;
// thanks both!

// UIController.h ////////////////////////////////////////////////////////////

#ifndef UNPHONE_UI0_H
#define UNPHONE_UI0_H
#if UNPHONE_UI0 == 1

#include "unPhone.h"
#include <WiFi.h>
#include <vector>

class UIElement { ///////////////////////////////////////////////////////////
  protected:
    Adafruit_HX8357* m_tft;
    XPT2046_Touchscreen* m_ts;
    SdFat* m_sd;

    // colour definitions
    const uint16_t BLACK =   HX8357_BLACK;
    const uint16_t BLUE =    HX8357_BLUE;
    const uint16_t RED =     HX8357_RED;
    const uint16_t GREEN =   HX8357_GREEN;
    const uint16_t CYAN =    HX8357_CYAN;
    const uint16_t MAGENTA = HX8357_MAGENTA;
    const uint16_t YELLOW =  HX8357_YELLOW;
    const uint16_t WHITE =   HX8357_WHITE;
    const uint8_t  BOXSIZE = 40;
    const uint8_t  HALFBOX = (BOXSIZE / 2);
    const uint8_t  QUARTBOX = (BOXSIZE / 4);
    const uint8_t  PENRADIUS = 9; // orig: 3
    static const uint8_t NUM_BOXES = 7;
    const uint16_t colour2box[NUM_BOXES] = {
      RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA, WHITE,
    };
    const uint8_t SWITCHER = 7; // index of the switcher
    void drawSwitcher(uint16_t xOrigin = 0, uint16_t yOrigin = 0);
    
  public:
    UIElement(Adafruit_HX8357* tft, XPT2046_Touchscreen* ts, SdFat *sdp);
    virtual bool handleTouch(long x, long y) = 0;
    virtual void draw() = 0;
    virtual void runEachTurn() = 0;
    void someFuncDummy();
    void showLine(const char *buf, uint16_t *yCursor);
};

// the UI elements types (screens) /////////////////////////////////////////
enum ui_modes_t {
  ui_menu = 0,          //  0
  ui_configure,         //  1 (home)
  ui_wifi               //  2 (wifi)
};

class UIController { ////////////////////////////////////////////////////////
  private:
    UIElement* m_element = 0;
    UIElement* m_menu;
    bool gotTouch();
    void handleTouch();
    void changeMode();
    ui_modes_t m_mode;                  // current mode (aka screen, element)
    ui_modes_t nextMode = ui_configure; // starting mode
  public:
    UIController(ui_modes_t);
    bool begin(unPhone& u);
    bool begin(unPhone& u, bool);
    UIElement* allocateUIElement(ui_modes_t);
    void run();
    void redraw();
    void message(char *s);
    static bool provisioned;
    const char *modeName(ui_modes_t);
    static const char *ui_mode_names[]; // a string name for each mode
    static uint8_t NUM_UI_ELEMENTS;     // number of UI elements (modes/scrns)
};


//  AllUIElement.h ///////////////////////////////////////////////////////////

class MenuUIElement: public UIElement { /////////////////////////////////////
  private:
    void drawTextBoxes();
    int8_t mapTextTouch(long, long);
    int8_t menuItemSelected = -1;
  public:
    MenuUIElement (Adafruit_HX8357* tft, XPT2046_Touchscreen* ts, SdFat* sd)
    : UIElement(tft, ts, sd) {
      // nothing to initialise
    };
    bool handleTouch(long x, long y);
    void draw();
    void runEachTurn();
    int8_t getMenuItemSelected() { return menuItemSelected; }
};

class ConfigUIElement: public UIElement { ///////////////////////////////////
  private:
    long m_timer;
  public:
    ConfigUIElement (Adafruit_HX8357* tft, XPT2046_Touchscreen* ts, SdFat* sd)
     : UIElement(tft, ts, sd) { m_timer = millis(); };
    bool handleTouch(long x, long y);
    void draw();
    void runEachTurn();
};

class WifiUIElement : public UIElement {
  private:
    int numberOfNetworks;
    std::vector<String> networkNames;
  
  public:
    WifiUIElement(Adafruit_HX8357* tft, XPT2046_Touchscreen* ts, SdFat* sd) : UIElement(tft, ts, sd) {}
    virtual void draw();
    virtual bool handleTouch(long x, long y);
    virtual void runEachTurn();
    void scanNetworks();
    void displayNetworks();
};


#endif // UNPHONE_UI0
#endif // header guard
