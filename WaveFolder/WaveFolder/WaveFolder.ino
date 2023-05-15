/*  Example demonstrating the WaveFolder class,
    using Mozzi sonification library.

    The WaveFolder class is used to fold a sine table several times

    Circuit: Audio output on digital pin 9 on a Uno or similar, or
    DAC/A14 on Teensy 3.1, or
    check the README or http://sensorium.github.io/Mozzi/

    Mozzi documentation/API
    https://sensorium.github.io/Mozzi/doc/html/index.html

    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users

    Combriat Thomas 2022, CC by-nc-sa.
*/


#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <tables/saw512_int8.h> // saw table for oscillator
#include <WaveFolder.h>

// analog freq pins
#define FUNDAMENTAL_PIN 0
#define BANDWIDTH_PIN 1
#define CENTREFREQ_PIN 2

// modulation pins analog
#define M1P 5
#define M2P 6
#define M3P 7
#define FLT_PIN 3

// button pins digita
#define B2_PIN 2
#define B3_PIN 3

#define SYNC_CONTROL         (4)

#define CONTROL_RATE 256 // powers of 2 please

// variables will change: 1-3
int buttonState = 0; 

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA); // audio oscillator
Oscil <SIN2048_NUM_CELLS, CONTROL_RATE> kBias(SIN2048_DATA);  // LFO changing the bias
Oscil <SAW512_NUM_CELLS, CONTROL_RATE> kGain(SAW512_DATA);  // Gain oscillator


// Create a WaveFolder on the native audio type (int).
// The template argument can be used to increase (or reduce)
// the data type this class is working on (affects performances)
WaveFolder<> wf;
int8_t gain = 0;
int8_t bias;

void setup() {
    // initialize the pushbutton pin as an input:
  pinMode(B2_PIN, INPUT);
  pinMode(B3_PIN, INPUT);
  pinMode(13, OUTPUT);

  aSin.setFreq(440); // set the frequency of the audio oscillator
  kBias.setFreq(0.3f); // set the frequency
  kGain.setFreq(0.2f);
  wf.setLimits(-2047, 2047);  // sets the values the wavefolder will start folding (in this case, containing in 12 bits)
                            // can also be done at CONTROL_RATE (even maybe at AUDIO_RATE)
  startMozzi(CONTROL_RATE);
}

void updateControl() {
  aSin.setFreq(mozziAnalogRead(FUNDAMENTAL_PIN) + mozziAnalogRead(M1P));
  bias = kBias.next()+ map(mozziAnalogRead(BANDWIDTH_PIN), 0,1023, 0.1f, 0.6f) + map(mozziAnalogRead(M2P),  0, 1023, 0.1f, 1.2f) ;
  gain = (kGain.next()>>1)+ map(mozziAnalogRead(CENTREFREQ_PIN), 0,1023, 24, 256) + map(mozziAnalogRead(M3P),  0, 1023, 8, 256) ;  
}

AudioOutput_t updateAudio() {
  int sample = (gain * aSin.next() >> 1) + (bias<<4);  // this is 8 + 7 - 1 + 1 = 15bits maximum.
                                                      // the wavefolder, set on an output of 12 bits will fold the exceeding values.
  return MonoOutput::fromNBit(12, wf.next(sample));   // return an int signal centred around 0
}

void loop() {
  audioHook(); // required here
}
