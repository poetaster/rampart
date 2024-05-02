/*  Example of playing sampled sounds,
    using Mozzi sonification library.

    Demonstrates one-shot samples scheduled
    with EventDelay(), and fast random numbers with
    xorshift96() and rand(), a more friendly wrapper for xorshift96().

    Circuit: Audio output on digital pin 9 on a Uno or similar, or
    DAC/A14 on Teensy 3.0/3.1, or
    check the README or http://sensorium.github.com/Mozzi/

    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users

    Tim Barrass 2012, CC by-nc-sa.


*/

//#include <ADC.h>  // Teensy 3.0/3.1 uncomment this line and install http://github.com/pedvide/ADC
#include <Mozzi.h>
#include <Sample.h> // Sample template
#include <samples/bamboo/bamboo_00_2048_int8.h> // wavetable data
#include <samples/bamboo/bamboo_01_2048_int8.h> // wavetable data
#include <samples/bamboo/bamboo_02_2048_int8.h> // wavetable data

#include "closedhat.h"
#include "kick.h"
#include "rim.h"
#include "snare.h"
#include "tom.h"
#include "SNARE2_AT.h"
#include "TR_HH_AT.h"
#include "TR_KICK_AT.h"
#include "KICK2_AT.h"
/*
  Sample <TR_HH_NUM_CELLS, AUDIO_RATE>aBamboo1(TR_HH_DATA);
  Sample <rim_NUM_CELLS, AUDIO_RATE>aBamboo0(rim_DATA);
  Sample <kick_NUM_CELLS, AUDIO_RATE>aBamboo2(kick_DATA);

   use: Sample <table_size, update_rate> SampleName (wavetable)
  Sample <SNARE2_NUM_CELLS, AUDIO_RATE>aBamboo0(SNARE2_DATA);
  Sample <TR_KICK_NUM_CELLS, AUDIO_RATE>aBamboo2(TR_KICK_DATA);
  Sample <KICK2_NUM_CELLS, AUDIO_RATE>aBamboo2(KICK2_DATA);

Sample <tom_NUM_CELLS, AUDIO_RATE>aBamboo2(tom_DATA);
Sample <BAMBOO_00_2048_NUM_CELLS, AUDIO_RATE>aBamboo0(BAMBOO_00_2048_DATA);
Sample <BAMBOO_01_2048_NUM_CELLS, AUDIO_RATE>aBamboo1(BAMBOO_01_2048_DATA);
Sample <BAMBOO_02_2048_NUM_CELLS, AUDIO_RATE>aBamboo2(BAMBOO_02_2048_DATA);

*/

Sample <rim_NUM_CELLS, AUDIO_RATE>aBamboo0(rim_DATA);
Sample <tom_NUM_CELLS, AUDIO_RATE>aBamboo1(tom_DATA);
Sample <closedhat_NUM_CELLS, AUDIO_RATE>aBamboo2(closedhat_DATA);



// STL stuff
#include <ArduinoSTL.h>
//#include <Arduino_AVRSTL.h>
using namespace std;
#include "bjorklung.h"
#include <iostream>
#include <string>
#include <vector>
#include <deque>

#include <EventDelay.h>
#include <mozzi_rand.h>

#define CONTROL_RATE 1024

// for scheduling audio gain changes
EventDelay kTriggerDelay;

std::string seq;

int currentbeat = 0;
int currentbeats = 0;
int patlen = 0;

// Button handling
const int BPIN = 4;
const int BPIN2 = 12;

// would be nice but need to design around it. nano only allows intr on d2 & d3 which I'm using. sigh.
//volatile unsigned long lastTime = 0;

// Used for timing clk

int marker = 13;   // marker output pin
int aval = 0;      // analog value

//patterns are now NOT external, see the bjorklung calcs for pattern generation

std::vector <int> beat = {1, 1, 1, 2, 1, 1, 1, 2};
//std::vector <int> beat = {1, 2, 1, 1, 2, 1, 1, 2};
std::vector <int> waf1 = {1, 1, 2}; // = (12) (West Africa, Latin America, N
std::vector <int> clas1 =  {1, 2, 1, 2, 2}; //= (23) (classical music, jazz,
std::vector <int> clas2 =  {1, 2, 2, 1, 2, 2, 2}; //  = (34) (classical music)0
std::vector <int> braz1 =  {1, 1, 1, 2}; // = (112) (Brazil, Bali rhythms), (Co
std::vector <int> tuar1 = {1, 2, 2, 1, 2, 2, 1, 2, 2, 2}; // = (334) (Tuareg
std::vector <int> grec1 =  {1, 1, 1, 1, 2}; // = (1112) (Greece)0,
std::vector <int> grec2 = {1, 2, 1, 2, 1, 2, 1, 2, 2} ; // = (2223) (Greece, Ma
std::vector <int> bulg1 =  {1, 2, 1, 2, 1, 2, 2}; // = (223) (Bulgaria, Greece,
std::vector <int> arab1 =  {1, 1, 1, 1, 1, 2}; // = (11112) (Arab)0,


// measure time
std::deque <int>offOne;

unsigned long startD;
unsigned long stopD;
int mills;
int millsA;
int millsB;

void setup() {
  // for clock in
  // attachInterrupt(6, readTime, RISING);

  // mozzi start
  startMozzi(CONTROL_RATE);

  Serial.begin(9600);


  // these were for testing generated sequences
  // seq = bjorklund(5, 8);
  // cout << seq << endl;

  // initial speed
  mills = 127;

  //aBamboo0.setFreq((float) BAMBOO_00_2048_SAMPLERATE / (float) BAMBOO_00_2048_NUM_CELLS);
  //aBamboo1.setFreq((float) BAMBOO_01_2048_SAMPLERATE / (float) BAMBOO_01_2048_NUM_CELLS);
  //aBamboo2.setFreq((float) BAMBOO_02_2048_SAMPLERATE / (float) BAMBOO_02_2048_NUM_CELLS);
  //aBamboo1.setFreq((float) TR_HH_SAMPLERATE / (float) TR_HH_NUM_CELLS);
  aBamboo0.setFreq((float) rim_SAMPLERATE / (float) rim_NUM_CELLS);
  aBamboo1.setFreq((float) tom_SAMPLERATE / (float) tom_NUM_CELLS);
  aBamboo2.setFreq((float) closedhat_SAMPLERATE / (float) closedhat_NUM_CELLS);
  
  /*
    aBamboo0.setFreq((float) SNARE2_SAMPLERATE / (float) SNARE2_NUM_CELLS); // play at the speed it was recorded at
    aBamboo1.setFreq((float) TR_HH_SAMPLERATE / (float) TR_HH_NU  M_CELLS);
    aBamboo2.setFreq((float) TR_KICK_SAMPLERATE / (float) TR_KICK_NUM_CELLS);
    aBamboo2.setFreq((float) snare_SAMPLERATE / (float) snare_NUM_CELLS);
    aBamboo0.setFreq((float) kick_SAMPLERATE / (float) kick_NUM_CELLS);
    aBamboo2.setFreq((float) KICK2_SAMPLERATE / (float) KICK2_NUM_CELLS);
    aBamboo2.setFreq((float) TR_KICK_SAMPLERATE / (float) TR_KICK_NUM_CELLS);
    aBamboo1.setFreq((float) BAMBOO_01_2048_SAMPLERATE / (float) BAMBOO_01_2048_NUM_CELLS); // play at the speed it was recorded at
    aBamboo1.setFreq((float) TR_HH_SAMPLERATE / (float) TR_HH_NUM_CELLS);
    aBamboo1.setFreq((float) BAMBOO_01_2048_SAMPLERATE / (float) BAMBOO_01_2048_NUM_CELLS);
    aBamboo2.setFreq((float) BAMBOO_02_2048_SAMPLERATE / (float) BAMBOO_02_2048_NUM_CELLS);
  */

  kTriggerDelay.set(111); // countdown ms, within resolution of CONTROL_RATE
  //Serial.println( bjorklund(13, 13));

  // Setup the first button with an internal pull-up :
  pinMode(BPIN, INPUT_PULLUP);
  pinMode(BPIN2, INPUT_PULLUP);

  // put some valuesin our timer offset deque
  offOne.push_front(256);
  offOne.push_front(256);
  offOne.push_front(256);

}

/* unused, might go there when we set up the layout differently.
  void readTime() {
    unsigned long t = millis();
    // calculate speed basing on t - lastTime
    lastTime = t;
  } */

byte randomGain(int amount) {
  //return lowByte(xorshift96())<<1;
  return rand(amount) ;
}

// referencing members from a struct is meant to be a bit faster than seperately
// ....haven't actually tested it here...
struct gainstruct {
  byte gain0;
  byte gain1;
  byte gain2;
} gains;




void updateControl() {


  int offsetT;
  offsetT = map(mozziAnalogRead(A2), 0, 1023, 1, 8);

  static int lastTrig;
  static int offset;

  // here we're timing a pulse in for clock
  int trig = map(mozziAnalogRead(A3), 0, 1023, 32, 1023);
  offset = map(mozziAnalogRead(A3), 0, 1023, 32, 1023);
  // set offset to be free running on CV or controlled by pot

  static int previousB;

  int currentB = digitalRead(BPIN2);

  if (previousB == LOW && currentB == HIGH) {
    offsetT += 1;
    Serial.println(currentbeats);
  } else if ( offsetT == 10 ) {
    offsetT = 1;
  }

  previousB = currentB;



  if ( lastTrig == 1023 && trig == 1023 ) { // sustained pulse

    mills += 1;
    //startD += micros(); // this is just entropy
  } else if ( lastTrig == 0 && trig == 0 && mills != 0 ) { // edge to 0

    mills = mills / 2;
    // this is smoothing
    millsB = millsA;
    millsA = mills;
    int millsMin = (millsA, millsB);
    millsMin = (millsMin, mills);
    offset = millsMin  * offsetT;
    //offset = startD  * offsetT;

    Serial.println("Second");

    Serial.println(offsetT);
    //Serial.println(mills);
    Serial.println(offset);
    //Serial.println(*it);

    mills = 0;
    startD = 0;
  }
  //Serial.println(lastTrig);
  lastTrig = trig;

  //just in case to keep the machine from hanging.
  if ( offset < 20 ) {
    offset = 256;
  }

  int beatval;
  //cout << currentbeats << endl;





  static int previous;
  int current = digitalRead(BPIN);
  if (previous == LOW && current == HIGH && currentbeats < 11) {
    currentbeats += 1;
    //Serial.println(currentbeats);
  }
  previous = current;

  if (currentbeats == 10) {
    currentbeats = 0;  //wrap pattern
  }

  switch (currentbeats) {
    case 0:
      patlen = beat.size();
      if (currentbeat > patlen) {
        currentbeat = 0;
      }
      beatval = beat[currentbeat];
      break;
    case 1:
      patlen = waf1.size();
      if (currentbeat > patlen) {
        currentbeat = 0;
      }
      beatval = waf1[currentbeat];
      break;
    case 2:
      patlen = clas1.size();
      if (currentbeat > patlen) {
        currentbeat = 0;
      }
      beatval = clas1[currentbeat];
      break;
    case 3:
      patlen = clas2.size();
      if (currentbeat > patlen) {
        currentbeat = 0;
      }
      beatval = clas2[currentbeat];
      break;
    case 4:
      patlen = braz1.size();
      if (currentbeat > patlen) {
        currentbeat = 0;
      }
      beatval = braz1[currentbeat];
      break;
    case 5:
      patlen = tuar1.size();
      if (currentbeat > patlen) {
        currentbeat = 0;
      }
      beatval = tuar1[currentbeat];
      break;
    case 6:
      patlen = grec1.size();
      if (currentbeat > patlen) {
        currentbeat = 0;
      }
      beatval = grec1[currentbeat];
      break;
    case 7:
      patlen = grec2.size();
      if (currentbeat > patlen) {
        currentbeat = 0;
      }
      beatval = grec2[currentbeat];
      break;
    case 8:
      patlen = bulg1.size();
      if (currentbeat > patlen) {
        currentbeat = 0;
      }
      beatval = bulg1[currentbeat];
      break;
    case 9:
      patlen = arab1.size();
      if (currentbeat > patlen) {
        currentbeat = 0;
      }
      beatval = arab1[currentbeat];
      break;

  }

  /* we're using a bit of arbitrary current beat switching to vary the rythms */

  if (kTriggerDelay.ready()) {
    if (beatval == 2) {
      if ( currentbeat == 0) {
        gains.gain1 = map(mozziAnalogRead(A1), 0, 1023, 0, 200) + randomGain(5);
        aBamboo1.start();
      } else {
        gains.gain0 = map(mozziAnalogRead(A0), 0, 1023, 0, 200) + randomGain(5);
        aBamboo0.start();
      }

    }
    if (beatval == 1) {
      if ( currentbeat == 3) {
        gains.gain1 = map(mozziAnalogRead(A1), 0, 1023, 0, 200) + randomGain(5);
        aBamboo1.start();
      } else {
        gains.gain2 = map(mozziAnalogRead(A0), 0, 1023, 0, 200) + randomGain(5);
        aBamboo2.start();
      }

    }

    /* More arbitrary variation, this time on gain */

    if (currentbeat = patlen) {
      switch (rand(0, 3)) {
        case 0:
          gains.gain0 = map(mozziAnalogRead(A0), 0, 1023, 0, 200) + randomGain(5);
          aBamboo0.start();
          break;
        case 1:
          gains.gain0 = map(mozziAnalogRead(A1), 0, 1023, 0, 200) + randomGain(5);
          aBamboo0.start();
          break;
        case 2:
          gains.gain1 = map(mozziAnalogRead(A2), 0, 1023, 0, 200) + randomGain(5);
          aBamboo1.start();
          break;

      }
    }
    /* More arbitrary variation, this time on gain
          switch(rand(0, 3)) {
          case 0:
          gains.gain0 = map(mozziAnalogRead(A0),0,1023,0,200) + randomGain();
          aBamboo0.start();
          break;
          case 1:
          gains.gain1 = map(mozziAnalogRead(A1),0,1023,0,200)+ randomGain();
          aBamboo1.start();
          break;
          case 2:
          gains.gain2 = map(mozziAnalogRead(A2),0,1023,0,200)+ randomGain();
          aBamboo2.start();
          break;
          } }
    */


    kTriggerDelay.set( offset ); // was + trig
    kTriggerDelay.start();

    //static int previousT;
    //int currentT = digitalRead(BPIN);

    currentbeat += 1;
  }


}

AudioOutput_t updateAudio() {

  int asig = (int)
             ((long) aBamboo0.next() * gains.gain0 +
              aBamboo1.next() * gains.gain1 +
              aBamboo2.next() * gains.gain2) >> 4;
  //clip to keep audio loud but still in range
  if (asig > 243) asig = 243;
  if (asig < -244) asig = -244;
  return asig;
}


void loop() {
  audioHook();

}


/*c old patterns


  = {1,0, 1,0, 1,0, 1,0, 1,0, 0,} = (22223) (clas
  = {1,0, 0, 1,0, 0, 1,0, 0, 1,0, 0, 1,0, 0, 0, 0
  {1,1,1,1,1,1,0,} = (111112) (Greek necklace)
  = {1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 0,} = (222223)
  {1,1,1,1,1,1,1,0,} = (1111112) (Libyan necklac
  = {1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 0,} = (22
  = {1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 0,}
  owing Euclidean rhythms are reverse Euclidean s
  {1,0, 1,0, x} = (221) (India), (Korean, Rumani
  {1,0, 0, 1,0, 0, 1,0,} = (332) (Central Africa
  = {1,0, 0, 0, 1,0, 0, 0, 1,0, 0,} = (443) (Nort
  = {1,0, 0, 0, 0, 1,0, 0, 0, 0, 1,0, 0, 0,} = (5
  {1,0, 1,0, 1,0, x} = (2221) (Bulgaria)0,
  = {1,0, 0, 1,0, 0, 1,0, 0, 1,0,} = (3332) (Sout
  = {1,0, 0, 0, 1,0, 0, 0, 1,0, 0, 0, 1,0, 0,} =
  {1,0, 1,1,0, 1,x} = (21211) (Arab)0,
  {1,0, 1,0, 1,0, 1,0, x} = (22221) (Arab)0,
  = {1,0, 0, 1,0, 1,0, 0, 1,0, 1,0,} = (32322) (M
  {1,0, 1,1,1,0, 1,1,x} = (2112111) (Greece)0,
  = {1,0, 1,1,0, 1,1,0, 1,x} = (2121211) (Turkey)
  = {1,0, 0, 1,0, 1,0, 1,0, 0, 1,0, 1,0, 1,0,} =
*/
