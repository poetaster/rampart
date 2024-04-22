/**************************************************************************
Rampart Bytebeats @copyright Mark Washeim blueprint@poetaster.de 

GPLv3

Some parts from https://raw.githubusercontent.com/schollz/nyblcore/main/bytebeat/bytebeat.ino
Some parts from https://github.com/spherical-sound-society/glitch-storm

Many contributions from the internet :) See nyblybyte.h for many equations origins and original form.
 **************************************************************************/
 

#include <EncoderButton.h>
//#include <Bounce2.h>
//#include <Adafruit_SSD1306.h> // conflicts with timer.
#include <EncoderButton.h>

// for pwm init functions
const unsigned int TOP = 0x07FF; // 11-bit resolution.  7812 Hz PWM


// from glitchstorm
#define LEDPIN   13 // usually 13
#define PWMPIN 11
#define UPPIN 3
#define DOWNPIN 2
#define progBit0Pin 7
#define progBit1Pin 6
#define progBit2Pin 5
#define progBit3Pin 4

int debounceRange = 20;// 5

long t = 0;
volatile int a, b, c,i;
volatile int value;
byte prog = 1;
byte upButtonState = 0;
byte downButtonState = 0;
byte lastButtonState = 0;
byte totalPrograms = 45;
byte clocksOut = 0;
int cyclebyte = 0;

// these ranges are provisional and in schollz equations need to be reset

volatile int aMax = 99;
volatile int aMin = 0;
volatile int bMax = 99;
volatile int bMin = 0;
volatile int cMax = 99;
volatile int cMin = 0;
int d = 0;

bool isClockOutMode = false;
bool isSerialValues = true;
unsigned long time_now = 0;

long button1Timer = 0;
long longPress1Time = 400;
long button2Timer = 0;
long longPress2Time = 400;

boolean isButton1Active = false;
boolean isLongPress1Active = false;
boolean isButton2Active = false;
boolean isLongPress2Active = false;

int  shift_A_Pot = 1;
int  old_A_Pot = 1;

int SRATE = 8192; // 16384;

byte shift_C_Pot = 0;
byte old_C_Pot = 0;

bool debug = true;

// encoder
// the a and b + the button pin
EncoderButton eb1(6,5,4);

int encoder_pos_last = 0;
long encoder_delta = 0;
int enc_offset = 1; // changes direction


#include "bytebeats.h"

// unused screen stuff. keep for now.

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//#include "font.h"
//#define myfont Org_01  // sigh
const int oled_sda_pin = 20;
const int oled_scl_pin = 21;
const int oled_i2c_addr = 0x3C;


// some constants for sound production
//counter for sound
float floatI = 0;
float tempo = 16;
int currentSound = 4;
int iterations = 256;

// analog freq pins
#define TEMPO 0
#define ITERATIONS 1

/**
 * handle encoder button long press event
 */
void onEb1LongPress(EncoderButton& eb) {
  
    if (debug) {
       Serial.print("button1 longPressCount: ");
       Serial.println(eb.longPressCount());
    }
}
/**
 * handle encoder turn with  button pressed
 * offsets OCR2A
 */
void onEb1PressTurn(EncoderButton& eb) {
  
  int dir = enc_offset + eb.increment();
  dir = constrain(dir, -7, 7 );
  enc_offset = dir;
  if (debug) {
    Serial.print("eb1 press inc by: ");
    Serial.println(eb.increment());
    Serial.print("enc_offset is: ");
    Serial.println(enc_offset);
  }
}

/**
 * handle encoder turn with  button pressed
 */
 
void onEb1Clicked(EncoderButton& eb) {
  Serial.print("eb1 clickCount: ");
  Serial.println(eb.clickCount());
   // displayUpdate();
}

/**
 * A function to handle the 'encoder' event without button
 */
void onEb1Encoder(EncoderButton& eb) {
  
  //displayUpdate();
  encoder_delta = eb.increment();
  long cstep = eb.increment() * 64;
  long oSRATE = SRATE;
  SRATE = SRATE + cstep;
  
  if (SRATE != oSRATE) {
    OCR1A = F_CPU / SRATE;
  }
  
  if (debug) {
    Serial.print("eb1 incremented by: ");
    Serial.println(eb.increment());
    Serial.print("eb1 position is: ");
    Serial.println(SRATE);
  }
}

long timeoffset = 0;

void setup() {

  if (debug) {
    Serial.begin(9600);
    Serial.println(F("Started"));
  }
 
  pinMode(LEDPIN, OUTPUT);
  pinMode(progBit0Pin, INPUT);
  //pinMode(progBit1Pin, OUTPUT);
  //pinMode(progBit2Pin, OUTPUT);
  pinMode(progBit3Pin, OUTPUT);

  pinMode(UPPIN, INPUT_PULLUP);
  pinMode(DOWNPIN, INPUT_PULLUP);
  
  pinMode(PWMPIN, OUTPUT);
  
  pwmSetup();
  
  //Link the event(s) to your function
  eb1.setClickHandler(onEb1Clicked);
  eb1.setEncoderHandler(onEb1Encoder);
  eb1.setLongPressHandler(onEb1LongPress, true);
  eb1.setEncoderPressedHandler(onEb1PressTurn);


}



void updateControl(){

   // this was old code from the midi-boy bytebeats sketch
  // tempo = map( analogRead(TEMPO), 0, 1023, 1, 256);
  // iterations = map( analogRead(ITERATIONS), 0, 1023, 16, 256);
     
  
  if (millis() > timeoffset + 50 ) {
      timeoffset = millis();
      //display_value(SRATE);
      if (value > 5000) digitalWrite(LEDPIN, HIGH);
      if (value < 1000) digitalWrite(LEDPIN, LOW);
  }
  
  // the sound selection is via encoder, so it's done heere
  eb1.update();
  
}



void loop(){

  
  buttonsManager();
  potsManager();
  adcManager();
  updateControl(); // required here

  
  

  
}

// used for mapping adc input on pins 5 - 7 to a - c
void adcManager() {

  if (!isButton1Active && !isButton2Active) {
    //a = map(analogRead(7), 0, 1023, aMin, aMax);
    //b = map(analogRead(6), 0, 1023, bMin, bMax);
    c =  (c + map(analogRead(7), 0, 1023, cMin, cMax))/2;

  }
}
void potsManager() {

  if (!isButton1Active && !isButton2Active) {
    a = map(analogRead(0), 0, 1023, aMin, aMax);
    // take the average on the input on pin 7
    b = map(analogRead(1), 0, 1023, bMin, bMax);
    c = map(analogRead(2), 0, 1023, cMin, cMax);

  }
  if (isLongPress2Active) {

    //left button is pressed
    //leftLongPressActions();


  }
  if (isLongPress1Active) {

    //right button is pressed
    rightLongPressActions();


  }

}
void rightLongPressActions() {

  //REVERSE TIME *********************
  int actual_A_Pot = map(analogRead(0), 0, 1023, -7, 7);
  if (old_A_Pot != actual_A_Pot) {
    shift_A_Pot = actual_A_Pot;
  }
  old_A_Pot = actual_A_Pot;
  if (shift_A_Pot == 0) {
    //prevents the engine to stop
    shift_A_Pot = 1;
  }
}

// show a number in binary on the LEDs
void display_value(int16_t value) {
  for (int i = 7; i >= 0; i--) { // NOPE + 1 can loop this way because port assignments are sequential
    //digitalWrite(led[i], value & 1);
    value = value >> 1;
  }
  //display_timer = millis();
}



int  softDebounce(int  readCV, int  oldRead) {
  if (abs(readCV - oldRead) > debounceRange) {
    return readCV;
  }
  return oldRead;
}

void buttonsManager() {
  bool pressBothButtons = false;
  
  //start button 1
  if (digitalRead(UPPIN) == LOW) {
    if (isButton1Active == false) {
      isButton1Active = true;
      button1Timer = millis();      
      if (debug) Serial.println("RIGHT button short press");
      
    }
    if ((millis() - button1Timer > longPress1Time) && (isLongPress1Active == false)) {
      isLongPress1Active = true;
      if (debug) Serial.println("RIGHT long press ON");
      
    }
  } else {
    if (isButton1Active == true) {
      if (isLongPress1Active == true) {
        isLongPress1Active = false;
        if (debug) Serial.println("RIGHT long press RELEASE");
      } else {

        if (prog != totalPrograms) {
          prog++;
        }
        else if (prog == totalPrograms) {
          prog = 1;
        } 
        if (debug) Serial.println("RIGHT button short release");
        if (debug) Serial.print("PROGRAM: ");
        if (debug) Serial.println(prog);
        
        
      }
      isButton1Active = false;
    }
  }
  //end RIGHT button
  //start LEFT button
  if (digitalRead(DOWNPIN) == LOW) {
    if (isButton2Active == false) {
      isButton2Active = true;
      button2Timer = millis();
      if (debug) Serial.println("LEFT button short press");
    }
    if ((millis() - button2Timer > longPress2Time) && (isLongPress2Active == false)) {
      isLongPress2Active = true;

      if (debug) Serial.println("LEFT BUTTON long press ON");
    }
  } else {
    if (isButton2Active == true) {
      if (isLongPress2Active == true) {
        isLongPress2Active = false;
        if (debug) Serial.println("LEFT BUTTON long press release");
        pressBothButtons = true;
        //isClockOutMode = !isClockOutMode;
        //we only change program in short pressed, not long ones
        prog++;

      } else {
        if (downButtonState == LOW) {
          if (prog > 1) {
            prog--;
          } else if (prog == 1) {
            prog = totalPrograms;
          }
          if (debug) Serial.println("LEFT BUTTON short release");
        }
        
        isButton2Active = false;
      }

    }
    //end button 2

    if (!isLongPress2Active && isLongPress1Active && pressBothButtons) {
      if (debug) Serial.println("HACKKK");
      isClockOutMode = !isClockOutMode;
    }
  }
}



// additional display methods (these don't clear)

//
// --- display details
//
typedef struct { int x; int y;  const char* str;} pos_t;
/*
//// {x,y} locations of play screen items
const int step_text_pos[] = { 0, 15, 16, 15, 32, 15, 48, 15, 64, 15, 80, 15, 96, 15, 112, 15 };
const pos_t bpm_text_pos    = {.x=0,  .y=15, .str="bpm:%3d" };
const pos_t trans_text_pos  = {.x=35, .y=15, .str="trs:%+2d" };
const pos_t seqno_text_pos  = {.x=75, .y=15, .str="seq:%d" };
const pos_t seq_info_pos    = {.x=60, .y=45, .str="" };
const pos_t play_text_pos   = {.x=110,.y=57, .str="" };

const pos_t oct_text_offset = { .x=3, .y=10,  .str="" };
const pos_t gate_bar_offset = { .x=0, .y=-15, .str="" };
const pos_t edit_text_offset= { .x=3, .y=22,  .str="" };
const int gate_bar_width = 14;
const int gate_bar_height = 4;

void displayUpdate() {
  display.clearDisplay();
  //display.setFont(&myfont);
  display.setFont(u8g2_font_ncenB14_tr);
  //display.setTextColor(WHITE, 0);
  //display.setFont(&myfont2);
  // bpm
  display.setCursor(bpm_text_pos.x, bpm_text_pos.y);
  display.print("inc: ");
  display.print(eb1.increment());

  // transpose
  display.setCursor(trans_text_pos.x, trans_text_pos.y);
    display.print("pos: ");
  display.print(eb1.position());

  // seqno
  display.setCursor(seqno_text_pos.x, seqno_text_pos.y);
  display.print("clks: ");
  display.print(eb1.clickCount());  // user sees 1-8
  // seq info / meta
  display.setCursor(seq_info_pos.x, seq_info_pos.y);
  display.print((String) a);

  // play/pause
  //display.setCursor(play_text_pos.x, play_text_pos.y);
  //display.print(seqr.playing ? " >" : "[]");

  display.display();
}



void testscrolltext(void) {
  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("scroll"));
  display.display();      // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(1000);
}

void print_cur(String Bytes, String Bytes2) {
  display.setCursor(seq_info_pos.x, seq_info_pos.y);
  display.print(Bytes);
  display.print(Bytes2);  // user sees 1-8
  display.display();
}
*/
