/*
   Rampart Bytebeats @copyright 2024, Mark Washeim <blueprint@poetaster.de>
   GPLv3, see LICENSE
   some parts use other licensces (nyblybyte.h is MIT)
   and some parts are Public Domain in as far as permissible by law

   a number of formulas https://raw.githubusercontent.com/schollz/nyblcore/main/bytebeat/bytebeat.ino

   Many contributions from the internet :) See nyblybyte.h for many equations origins and original form.
*/

//#include <EEPROM.h>
//#define USE_LGT_EEPROM_API

//#include <util/crc16.h>
#include <EncoderButton.h>

// Define EEPROM addresses to store variables, , 4 bytes per float (freq_target1, freq_target2)
//#define PROG_PB1_ADR       0
//#define PROG_PB2_ADR       4
//#define PROG_PB3_ADR       8
//#define BANK_ADR           9
//#define EEPROM_CRC_ADR     11
//#define EEPROM_DATA_LENGTH 10  // 10 bytes of EEPROM written (without crc)

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


// for the sine waves
unsigned int Acc[3];
unsigned long freq[3];
unsigned int tune, pitch;
byte pot[] = {A1, A2, A3};
// from levitation osc.


long t = 0;
volatile int a, b, c, i;
volatile int offA, offB, offC;
volatile int result;
int d = 0; // hmm?

int prog = 1;
int bank = 1;
int banktotal = 3;
int pb1 = 1;
int pb1total = 25;
int pb2 = 1;
int pb2total = 28;
int pb3 = 1;
int pb3total = 20;

int numProg = 73;

// these ranges are provisional and in schollz equations need to be reset
volatile int aMax = 99, aMin = 0, bMax = 99, bMin = 0, cMax = 99, cMin = 0;

// default rate close to the original bytebeat speed
int SRATE = 8192; // 16384;

bool debug = false;

// encoder
// the a and b + the button pin large encoders are 6,5,4
EncoderButton eb1(5, 6, 4);

// the library has nice button handling and uses bounce2
EncoderButton left(3); // pin 3 rampart
EncoderButton right(2); // pin 2 rampart

int encoder_pos_last = 0;
long encoder_delta = 0;
int enc_offset = 1; // changes direction
int enc_delta; // which direction


#include "bytebeats.h" // forwards

// some constants for sound production these are the old ones.
//counter for sound
float floatI = 0;
float tempo = 16;
int currentSound = 4;
int iterations = 256;

// analog freq pins OLD
#define TEMPO 0
#define ITERATIONS 1

long timeoffset = 0;

/**
   handle encoder button long press event
*/
void onEb1LongPress(EncoderButton& eb) {
  
  // SAVE all selected registers to EEPROM
  /*
  EEPROM.write(PROG_PB1_ADR, pb1);
  EEPROM.write(PROG_PB2_ADR, pb2);
  EEPROM.write(PROG_PB3_ADR, pb3);
  EEPROM.write(BANK_ADR, bank);
  */
  
  if (debug) {
    Serial.print("button1 longPressCount: ");
    Serial.println(eb.longPressCount());
  }
}
/**
   handle encoder turn with  button pressed
   offsets OCR2A
*/
void onEb1PressTurn(EncoderButton& eb) {
  enc_delta = eb.increment();

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
   handle encoder turn with  button pressed
*/
void onEb1Clicked(EncoderButton& eb) {

  // set which bank to select formulas from
  bank = eb.clickCount();

  if (debug) {
    Serial.print("bank: ");
    Serial.println(eb.clickCount());
  }
  // displayUpdate();
}

/**
    handle left button short release
*/
void onLeftReleased(EncoderButton& left) {

  if (bank == 1)
  {
    if (pb1 > 1) {
      pb1--;
    } else if (pb1 == 1) {
      pb1 = pb1total;
    }
    prog = pb1;
  } 
  else if (bank == 2) {
    if (pb2 > 1) {
      pb2--;
    } else if (pb2 == 1) {
      pb2 = pb2total;
    }
    prog = pb2;
  } 
  else if (bank == 3) {
    if (pb3 > 1) {
      pb3--;
    } else if (pb3 == 1) {
      pb3 = pb3total;
    }
    prog = pb3;
  }

  if (debug) {
    Serial.print("PROGRAM: ");
    Serial.println(prog);
  }
}

/**
    handle right button short release
*/
void onRightReleased(EncoderButton& right) {
  
  if (bank == 1)
  {
    if (pb1 < pb1total) {
      pb1++;
    } else if (pb1 == pb1total) {
      pb1 = 1;
    }
    prog = pb1;
  } 
  else if (bank == 2) {
    if (pb2 < pb2total) {
      pb2++;
    } else if (pb2 == pb2total) {
      pb2 = 1;
    }
    prog = pb2;
  } 
  else if (bank == 3) {
    if (pb3 < pb3total) {
      pb3++;
    } else if (pb3 == pb3total) {
      pb3 = 1;
    }
    prog = pb3;
  }

  if (debug) {
    Serial.print("PROGRAM: ");
    Serial.println(prog);
  }
}

/**
   A function to handle the 'encoder' event without button
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

// values to compare input on CV pins
int lastA = 0;
int lastB = 0;
int lastC = 0;

void setup() {

  if (debug) {
    Serial.begin(57600);
    Serial.println(F("Started"));
  }
  /*
  // Load selected registers from EEPROM
  EEPROM.get(PROG_PB1_ADR, pb1);
  EEPROM.get(PROG_PB2_ADR, pb2);
  EEPROM.get(PROG_PB3_ADR, pb3);
  EEPROM.get(BANK_ADR, bank);
  
  Serial.println(F("pb1"));
  Serial.println(pb1);
  
  if (pb1 < 1) pb1 = 1;
  if (pb2 < 1) pb2 = 1;
  if (pb3 < 1) pb3 = 1;
  if (bank < 1) bank = 1;
  */

  
  pinMode(LEDPIN, OUTPUT);
  pinMode(PWMPIN, OUTPUT);
  
  //lastA =  analogRead(A3);
  //lastB =  analogRead(A6);
  //lastC =  analogRead(A7);
  
  pwmSetup();

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

  // this was old code from the midi-boy bytebeats sketch
  // tempo = map( analogRead(TEMPO), 0, 1023, 1, 256);
  // iterations = map( analogRead(ITERATIONS), 0, 1023, 16, 256);
  
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
  // for the wave synth


  //if ( debug ) Serial.println(freq[0]);
  //if ( debug ) Serial.println(pgm_read_byte(&sine256[Acc[0] >> 8]) + pgm_read_byte(&sine256[Acc[1] >> 8]) + pgm_read_byte(&sine256[Acc[2] >> 8]));//Acc[0]>>8);




}

void loop() {
  updateControl(); // required here
  adc();
  knobs();

  
}



// used for mapping adc input on pins 5 - 7 to a - c
void adc() {
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
  a = map(analogRead(A0), 0, 1023, aMin, aMax) + offA;
  b = map(analogRead(A1), 0, 1023, bMin, bMax) + offB;
  c = map(analogRead(A2), 0, 1023, cMin, cMax) + offC;
}


// show a number in binary on the LEDs
void display_value(int16_t value) {
  for (int i = 7; i >= 0; i--) { // NOPE + 1 can loop this way because port assignments are sequential
    //digitalWrite(led[i], result & 1);
    value = value >> 1;
  }
  //display_timer = millis();
}
