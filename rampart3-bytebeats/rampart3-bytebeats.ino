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

// for pwm

const unsigned int TOP = 0x07FF; // 11-bit resolution.  7812 Hz PWM


// from glitchstorm
#define LEDPIN   13
#define PWMPIN 11
#define UPPIN 5
#define DOWNPIN 4
#define progBit0Pin 7
#define progBit1Pin 6
#define progBit2Pin 5
#define progBit3Pin 4

int debounceRange = 20;// 5

long t = 0;
volatile int a, b, c,i;
volatile int value;
byte programNumber = 1;
byte upButtonState = 0;
byte downButtonState = 0;
byte lastButtonState = 0;
byte totalPrograms = 39;
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

int SAMPLE_RATE = 8192; // 16384;
int old_SAMPLE_RATE = SAMPLE_RATE;

byte shift_C_Pot = 0;
byte old_C_Pot = 0;

bool debug = true;

#include "bytebeats.h"

//Bounce2::Button keys[numsteps];
//Bounce2::Button encoder_switch;

// INSTANTIATE A Bounce OBJECT
//Bounce bounce = Bounce();

//#include "font.h"

//#define myfont Org_01  // sigh


//RotaryEncoder encoder(encoderB_pin, encoderA_pin, RotaryEncoder::LatchMode::FOUR3);
//void checkEncoderPosition() {  encoder.tick();  } // call tick() to check the state.

int encoder_pos_last = 0;

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

EncoderButton eb1(3,2,4);

const int encoderA_pin = 18;
const int encoderB_pin = 19;
const int encoderSW_pin = 22;

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
void onEb1Clicked(EncoderButton& eb) {
  Serial.print("eb1 clickCount: ");
  Serial.println(eb.clickCount());
   // displayUpdate();
}

/**
 * A function to handle the 'encoder' event
 */
void onEb1Encoder(EncoderButton& eb) {

  //displayUpdate();
  currentSound = eb.increment() * 64;
  old_SAMPLE_RATE = SAMPLE_RATE;
  SAMPLE_RATE = SAMPLE_RATE + currentSound;
  if (SAMPLE_RATE != old_SAMPLE_RATE) {
    OCR1A = F_CPU / SAMPLE_RATE;
  }
  if (debug) {
    Serial.print("eb1 incremented by: ");
    Serial.println(eb.increment());
    Serial.print("eb1 position is: ");
    Serial.println(SAMPLE_RATE);
  }
}


void setup() {

  if (debug) {
    Serial.begin(9600);
    Serial.println(F("Started"));
  }

  
  
  pinMode(LEDPIN, OUTPUT);
  pinMode(1,OUTPUT);
  pinMode(progBit0Pin, INPUT);
  //pinMode(progBit1Pin, OUTPUT);
  //pinMode(progBit2Pin, OUTPUT);
  pinMode(progBit3Pin, OUTPUT);

  pinMode(UPPIN, INPUT_PULLUP);
  pinMode(DOWNPIN, INPUT_PULLUP);

  initSound();
  ledCounter();
  
  //Link the event(s) to your function
  eb1.setClickHandler(onEb1Clicked);
  eb1.setEncoderHandler(onEb1Encoder);

  
   //pinMode(9, OUTPUT); // set up pin 9 for PWM audio out

}

long timeoffset = 0;

void updateControl(){

  ledCounter(); /// could be in loop
  //floatI += tempo; //float counter so that you can set tempo
      // always write the sound out begore doing or not doing arduboy stuff
  i = i + tempo;
  if (i>256) i = 1;
  //a = getSound(currentSound, i);
  //print_cur("Sound: ", (String) a);
  if (debug) {
    //Serial.println(a);
  }
  if (value < 50 ) digitalWrite(13,LOW);
  if (value > 80 ) digitalWrite(13,HIGH);
  

  //tempo = map( analogRead(TEMPO), 0, 1023, 1, 256);
  //iterations = map( analogRead(ITERATIONS), 0, 1023, 16, 256);

  
  // the sound selection is via encoder, so it's done heere
  eb1.update();
  
}



void loop(){

  
  buttonsManager();
  potsManager();
  adcManager();
  updateControl(); // required here
    if (millis() > timeoffset + 100) {
      timeoffset = millis();
      //display.display();
        //analogWrite(13,value);

  }
}

// used for mapping adc input on pins 5 - 7 to a - c
void adcManager() {

  if (!isButton1Active && !isButton2Active) {
    a = map(analogRead(7), 0, 1023, aMin, aMax);
    //b = map(analogRead(1), 0, 1023, bMin, bMax);
    //c = map(analogRead(2), 0, 1023, cMin, cMax);

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


void ledCounter() {
  int val;
  if (isClockOutMode) {
    //show clocks
    clocksOut++;
    if (clocksOut == 16) {
      clocksOut = 0;
    }
    val = clocksOut;
  } else {
    //show program number in binary
    val = programNumber;
  }
//  digitalWrite(progBit0Pin, bitRead(val, 0));
//  digitalWrite(progBit1Pin, bitRead(val, 1));
//  digitalWrite(progBit2Pin, bitRead(val, 2));
  //digitalWrite(progBit3Pin, bitRead(val, 3));
  //Serial.println(val);
  //digitalWrite(progBit3Pin, value);
}

void printValues() {
  Serial.print("programNumber: ");
  Serial.println(programNumber);
  Serial.print("A: ");
  Serial.print(a);
  Serial.print(" B: ");
  Serial.print(b);
  Serial.print(" C: ");
  Serial.println(c);
  Serial.print(" APot: ");
  Serial.println(shift_A_Pot);
  Serial.print("rate: ");
  Serial.println(old_SAMPLE_RATE);

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

        if (programNumber != totalPrograms) {
          programNumber++;
        }
        else if (programNumber == totalPrograms) {
          programNumber = 1;
        } 
        if (debug) Serial.println("RIGHT button short release");
        if (debug) Serial.print("PROGRAM: ");
        if (debug) Serial.println(programNumber);
        
        ledCounter();
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
        programNumber++;

      } else {
        if (downButtonState == LOW) {
          if (programNumber > 1) {
            programNumber--;
          } else if (programNumber == 1) {
            programNumber = totalPrograms;
          }
          if (debug) Serial.println("LEFT BUTTON short release");
        }
        ledCounter();
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
