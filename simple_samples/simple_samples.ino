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
#include <MozziGuts.h>
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


// STL stuff
#include <ArduinoSTL.h>
using namespace std;
#include "bjorklung.h"
#include <iostream>
#include <string>
#include <vector>

#include <EventDelay.h>
#include <mozzi_rand.h>

#define CONTROL_RATE 256

// use: Sample <table_size, update_rate> SampleName (wavetable)
//Sample <SNARE2_NUM_CELLS, AUDIO_RATE>aBamboo0(SNARE2_DATA);
Sample <TR_HH_NUM_CELLS, AUDIO_RATE>aBamboo1(TR_HH_DATA);
//Sample <TR_KICK_NUM_CELLS, AUDIO_RATE>aBamboo2(TR_KICK_DATA);

Sample <rim_NUM_CELLS, AUDIO_RATE>aBamboo0(rim_DATA);

//Sample <kick_NUM_CELLS, AUDIO_RATE>aBamboo2(kick_DATA);

Sample <KICK2_NUM_CELLS, AUDIO_RATE>aBamboo2(KICK2_DATA);
//Sample <BAMBOO_01_2048_NUM_CELLS, AUDIO_RATE>aBamboo1(BAMBOO_01_2048_DATA);

//Sample <BAMBOO_01_2048_NUM_CELLS, AUDIO_RATE>aBamboo1(BAMBOO_01_2048_DATA);
//Sample <BAMBOO_02_2048_NUM_CELLS, AUDIO_RATE>aBamboo2(BAMBOO_02_2048_DATA);

// for scheduling audio gain changes
EventDelay kTriggerDelay;

std::string seq;

int currentbeat = 0;
int currentbeats = 0;
int patlen = 0;

// Button handling
const int buttonPin = 2;



void setup() {
  startMozzi(CONTROL_RATE);
  //Serial.begin(115200)
  Serial.begin(9600);
  seq = bjorklund(5, 8);
  cout << seq << endl;

  /*
    //aBamboo0.setFreq((float) SNARE2_SAMPLERATE / (float) SNARE2_NUM_CELLS); // play at the speed it was recorded at
    aBamboo1.setFreq((float) TR_HH_SAMPLERATE / (float) TR_HH_NUM_CELLS);
    aBamboo2.setFreq((float) TR_KICK_SAMPLERATE / (float) TR_KICK_NUM_CELLS);
    //aBamboo2.setFreq((float) snare_SAMPLERATE / (float) snare_NUM_CELLS);
    aBamboo0.setFreq((float) kick_SAMPLERATE / (float) kick_NUM_CELLS);
  */
  aBamboo2.setFreq((float) KICK2_SAMPLERATE / (float) KICK2_NUM_CELLS);
  aBamboo0.setFreq((float) rim_SAMPLERATE / (float) rim_NUM_CELLS);
  //aBamboo1.setFreq((float) TR_HH_SAMPLERATE / (float) TR_HH_NUM_CELLS);
  //aBamboo1.setFreq((float) BAMBOO_01_2048_SAMPLERATE / (float) BAMBOO_01_2048_NUM_CELLS); // play at the speed it was recorded at
  aBamboo1.setFreq((float) TR_HH_SAMPLERATE / (float) TR_HH_NUM_CELLS);
  //aBamboo1.setFreq((float) BAMBOO_01_2048_SAMPLERATE / (float) BAMBOO_01_2048_NUM_CELLS);
  //aBamboo2.setFreq((float) BAMBOO_02_2048_SAMPLERATE / (float) BAMBOO_02_2048_NUM_CELLS);

  kTriggerDelay.set(111); // countdown ms, within resolution of CONTROL_RATE
  //Serial.println( bjorklund(13, 13));

  // Setup the first button with an internal pull-up :
  pinMode(buttonPin, INPUT_PULLUP);

}


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
}
gains;



//patterns

/*c


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


std::vector <int> beat = {1, 2, 1, 1, 2, 1, 1, 2};
std::vector <int> waf1 = {1, 1, 2}; // = (12) (West Africa, Latin America, N
std::vector <int> clas1 =  {1, 2, 1, 2, 2}; //= (23) (classical music, jazz,
std::vector <int> clas2 =  {1, 2, 2, 1, 2, 2, 2}; //  = (34) (classical music)0
std::vector <int> braz1 =  {1, 1, 1, 2}; // = (112) (Brazil, Bali rhythms), (Co
std::vector <int> tuar1 = {1, 2, 2, 1, 2, 2, 1, 2, 2, 2}; // = (334) (Tuareg
std::vector <int> grec1 =  {1, 1, 1, 1, 2}; // = (1112) (Greece)0,
std::vector <int> grec2 = {1, 2, 1, 2, 1, 2, 1, 2, 2} ; // = (2223) (Greece, Ma
std::vector <int> bulg1 =  {1, 2, 1, 2, 1, 2, 2}; // = (223) (Bulgaria, Greece,
std::vector <int> arab1 =  {1,1,1,1,1,2}; // = (11112) (Arab)0,

void updateControl() {
  int beatval;
  //cout << currentbeats << endl;

  static int previous;
  int current = digitalRead(buttonPin);
  if (previous == LOW && current == HIGH && currentbeats < 11) {
    currentbeats += 1;
    Serial.println(currentbeats);
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


  if (kTriggerDelay.ready()) {
    if (beatval == 2) {
      gains.gain0 = map(mozziAnalogRead(A0), 0, 1023, 0, 200) + randomGain(5);
      aBamboo0.start();
    }
    if (beatval == 1) {
      gains.gain2 = map(mozziAnalogRead(A1), 0, 1023, 0, 200) + randomGain(5);
      aBamboo2.start();


      /*if (currentbeat = patlen) {
        switch (rand(0, 3)) {
          case 0:
            gains.gain0 = map(mozziAnalogRead(A1), 0, 1023, 0, 200) + randomGain(5);
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
        }*/

    }

    /*
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
      } }*/
    int trig = map(mozziAnalogRead(A4), 0, 1023, 1, 100);
    int offset = map(mozziAnalogRead(A3), 0, 1023, 25, 250);
    
    kTriggerDelay.set( trig + offset );
      
    kTriggerDelay.start();
    //static int previousT;
    //int currentT = digitalRead(buttonPin);
  
    
    Serial.println( trig + offset);

    currentbeat += 1;
  }


}


int updateAudio() {
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
