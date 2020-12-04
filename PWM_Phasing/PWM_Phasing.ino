/*  Example of pulse width modulation,
    using Mozzi sonification library.
     
    Based Miller Puckette's j03.pulse.width.mod example 
    in the Pure Data documentation, subtracting 2 sawtooth 
    waves with slightly different tunings to produce a 
    varying phase difference.
  
    Demonstrates Phasor().
  
    Circuit: Audio output on digital pin 9 on a Uno or similar, or
    DAC/A14 on Teensy 3.0/3.1, or 
    check the README or http://sensorium.github.com/Mozzi/
  
    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users
  
    Tim Barrass 2012, CC by-nc-sa.
*/

//#include <ADC.h>  // Teensy 3.0/3.1 uncomment this line and install http://github.com/pedvide/ADC
#include <MozziGuts.h>
#include <Phasor.h>
#include <RollingAverage.h>

#define CONTROL_RATE 64 // powers of 2 please

Phasor <AUDIO_RATE> aPhasor1;
Phasor <AUDIO_RATE> aPhasor2;


// analog freq pins
#define F_PIN 0
#define B_PIN 1
#define C_PIN 2

// modulation pins analog
#define M1_PIN 5
#define M1_PIN 6
#define M1_PIN 7
#define FLT_PIN 3

// button pins digita
#define B2_PIN 2
#define B3_PIN 3


// for smoothing the control signals
// use: RollingAverage <number_type, how_many_to_average> myThing
RollingAverage <int, 32> kAverageF;
RollingAverage <int, 32> kAverageBw;
RollingAverage <int, 32> kAverageM1;
RollingAverage <int, 32> kAverageM2;

//same variables

float freq = 55.f;
float freq2 = 55.f;

void setup(){
  aPhasor1.setFreq(freq);
  aPhasor2.setFreq(freq+0.2f);  
  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)
}


void updateControl(){
  freq = kAverageF.next( mozziAnalogRead(F_PIN)) + kAverageM1.next(mozziAnalogRead(A5) ) / 2;
  freq2 = kAverageBw.next(mozziAnalogRead(B_PIN) ) + kAverageM2.next(mozziAnalogRead(A6)) /2;
  aPhasor1.setFreq(freq);
  aPhasor2.setFreq(freq2);
}


int updateAudio(){
  char asig1 = (char)(aPhasor1.next()>>24);
  char asig2 = (char)(aPhasor2.next()>>24);
  return ((int)asig1-asig2)/2;
}


void loop(){
  audioHook(); // required here
}
