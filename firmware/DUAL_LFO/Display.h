#include "Arduino.h"

#include <Adafruit_SSD1306.h>

//OLED display setting
#include <SPI.h>               //for OLED display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Flag for using module upside down
#define PANEL_USD

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

class Display {

    public:

    Display();
    void begin();
    void update();
    
};
