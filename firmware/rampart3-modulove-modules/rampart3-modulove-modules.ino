/*
    Rampart Modulove @copyright 2025, Mark Washeim <blueprint@poetaster.de>
    GPLv3, see LICENSE
    Much of the code is
   @author Adam Wonak (https://github.com/awonak/)

   @copyright Copyright (c) 2023

   some parts use other licensces (see the header files is MIT)
   and some parts are Public Domain in as far as permissible by law

*/


bool debug = false;

// slightly modified from the original
#include "synclfo.h"
// Declare SyncLFO hardware variable.
SyncLFO hw;
#include "Polyrythm.h"
#include "GenerativeSequencer.h"
#include "MultimodeEnv.h"
#include "adsr.h"
#include "BurstGenerator.h"

#include <EncoderButton.h>
// encoder
// the a and b + the button pin large encoders are 6,5,4
EncoderButton eb1(5, 6, 4);

// the library has nice button handling and uses bounce2
EncoderButton left(3); // pin 3 rampart
EncoderButton right(2); // pin 2 rampart



// encoder + button logic
#include "encoderbutton.h"

// for pwm init functions
const unsigned int TOP = 0x07FF; // 11-bit resolution.  7812 Hz PWM

// noise reduction for cv reads
uint16_t potvalue[3]; // pot readings
uint16_t lastpotvalue[3]; // old pot readings
#define POT_SAMPLE_TIME 30 // delay time between pot reads
#define MIN_POT_CHANGE 25 // locked pot reading must change by this in order to register
#define MIN_COUNTS 20  // unlocked pot must change by this in order to register
#define POT_AVERAGING 3 // analog sample averaging count 
#define POT_MIN 4   // A/D may not read min value of 0 so use a bit larger value for map() function
#define POT_MAX 1019 // A/D may not read max value of 1023 so use a bit smaller value for map() function

#define LEDPIN   13 // usually 13
#define PWMPIN 11



long t = 0;
volatile int a, b, c;
volatile int offA, offB, offC;
volatile int result;
int d = 0; // hmm?







long timeoffset = 0;


// values to compare input on CV pins
int lastA = 0;
int lastB = 0;
int lastC = 0;

void setup() {

  if (debug) {
    Serial.begin(57600);
    Serial.println(F("Started"));
  }
  // Initialize the SyncLFO peripherials.
  // adapted to rampart
  hw.Init();
  setupPoly() ; // setup the Polyrythm generator
  setupGenSeq(); // setup the generative sequencer

  pinMode(LEDPIN, OUTPUT);
  pinMode(PWMPIN, OUTPUT);

  //lastA =  analogRead(A3);
  //lastB =  analogRead(A6);
  //lastC =  analogRead(A7);



  //Link the event(s) to your function
  eb1.setClickHandler(onEb1Clicked);
  eb1.setEncoderHandler(onEb1Encoder);
  eb1.setLongPressHandler(onEb1LongPress, true);
  eb1.setEncoderPressedHandler(onEb1PressTurn);

  // program up/down buttons
  left.setReleasedHandler(onLeftReleased);
  left.setRateLimit(7);
  right.setReleasedHandler(onRightReleased);
  right.setRateLimit(6);

}


void updateControl() {


  // EncoderButton object updates
  eb1.update();
  left.update();
  right.update();

  if (millis() > timeoffset + 50 ) {
    timeoffset = millis();
    //display_value(SRATE);
    if (result > 5000) digitalWrite(LEDPIN, HIGH);
    if (result < 1000) digitalWrite(LEDPIN, LOW);

  }



}

void loop() {
  updateControl(); // required here
  //adc();
  //knobs();
  
  if (bank == 1) {
    loopPoly();
  } else if (bank == 2) {
    loopGenSeq();
  } else if (bank == 3) {
    loopMMenv();
  } else if (bank == 4) {
    loopADSR();
  } else if (bank ==5) {
    loopBrstGen();
  }


}



// used for mapping adc input on pins 5 - 7 to a - c
void adc() {

  uint16_t aMin = 0, aMax = 100;
  // this is just very wrong ;)
  // take the average on the input on pin 3,6,7
  uint16_t A =  map(readcv(0), 0, 1023, aMin, aMax);
  uint16_t B =  map(readcv(1), 0, 1023, aMin, aMax);
  uint16_t C =  map(readcv(2), 0, 1023, aMin, aMax);

  if (abs(lastA - A) > 1) {
    lastA = A;
    offA = lastA / 2;
    if (debug) Serial.println(A);

  }

  if (abs(lastB - B) > 1) {
    lastB = B;
    offB = lastB / 2;

  }

  if (abs(lastC - C) > 1) {
    lastC = C;
    offC = lastC / 2 ;

  }

}

uint16_t readcv(uint8_t potnum) {
  int val = 0;
  int input;
  switch (potnum) { // map potnum to pin
    case 0:
      input = A3;
      break;
    case 1:
      input = A6;
      break;
    case 2:
      input = A7;
      break;
  }
  // note that Pikocore pots are wired "backwards" - max voltage is full ccw
  for (int j = 0; j < POT_AVERAGING; ++j) val += (analogRead(input)); // read the A/D a few times and average for a more stable value
  val = val / POT_AVERAGING;

  if (abs(lastpotvalue[potnum] - val) > MIN_COUNTS ) {
    lastpotvalue[potnum] = val; // even if pot is unlocked, make sure pot has moved at least MIN_COUNT counts so values don't jump around
  }  else {
    val = lastpotvalue[potnum];
  }

  potvalue[potnum] = val; // pot is unlocked so save the reading

  return val;
}

// pot inputs
void knobs() {
  /*
    a = map(analogRead(A0), 0, 1023, aMin, aMax) + offA;
    b = map(analogRead(A1), 0, 1023, bMin, bMax) + offB;
    c = map(analogRead(A2), 0, 1023, cMin, cMax) + offC;
  */
}


// show a number in binary on the LEDs
void display_value(int16_t value) {
  for (int i = 7; i >= 0; i--) { // NOPE + 1 can loop this way because port assignments are sequential
    //digitalWrite(led[i], result & 1);
    value = value >> 1;
  }
  //display_timer = millis();
}
