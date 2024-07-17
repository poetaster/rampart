/*  Example of Wavepacket synthesis, using Mozzi sonification library.
    This sketch draws on Miller Puckette's 
    Pure Data example, F14.wave.packet.pd, with
    two overlapping streams of wave packets.
  
    Circuit:
    	Audio output on DAC/A14 on Teensy 3.0, 3.1, 
    	or digital pin 9 on a Uno or similar, or 
    	check the README or http://sensorium.github.com/Mozzi/
  
    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users
  
    Tim Barrass 2013, CC by-nc-sa.
*/
//#include <VS1053Driver.h>
//#include <ADC.h>  // Teensy 3.0/3.1 uncomment this line and install http://github.com/pedvide/ADC
#include <MozziConfigValues.h>
//#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_2PIN_PWM

#include <Mozzi.h>
#include <mozzi_analog.h>
#include <WavePacket.h>
#include <RollingAverage.h>
#include <LowPassFilter.h>

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include <EventDelay.h>

// from PDresonant
#include <IntMap.h>
#include "PDResonant.h"
// wavetable for oscillator:
#include <tables/sin2048_int8.h>
PDResonant voice;

// for fake midi
EventDelay startNote; 
EventDelay endNote;

// for FMsynth
#include <Oscil.h>
#include <tables/cos2048_int8.h> // table for Oscils to play



// Reso! analog joystick for controlling speed of modulation: assigned to attack, decay times and sustain level
#define X A0
#define Y A1
unsigned int x_axis = 512; // give a static value for test without midi
unsigned int y_axis = 512;
const IntMap kmapX(0, 1023, 0, 500); // A
const IntMap kmapY(0, 1023, 0, 1000); //D

// analog freq pins
#define FUNDAMENTAL_PIN 0
#define BANDWIDTH_PIN 1
#define CENTREFREQ_PIN 2

// modulation pins analog
#define M1_PIN 5
#define M1_PIN 6
#define M1_PIN 7
#define FLT_PIN 3

// Button handling
const int BPIN = 4;
const int BPIN2 = 3;

// Map Analogue channels
#define SYNC_CONTROL         (4)

#define CONTROL_RATE 256 // powers of 2 please

// for smoothing the control signals
// use: RollingAverage <number_type, how_many_to_average> myThing
RollingAverage <int, 32> kAverageF;
RollingAverage <int, 32> kAverageBw;
RollingAverage <int, 32> kAverageCf;
RollingAverage <int, 32> kAverageM1;
RollingAverage <int, 32> kAverageM2;
RollingAverage <int, 32> kAverageM3;
WavePacket <SINGLE> wavey; // <DOUBLE> wavey; // DOUBLE selects 2 overlapping streams


bool debug = true;


// variables will change: 1-3
int buttonState = 0;

#include "encoder.h"


void setup(){
  
  Serial.begin(9600);
  // initialize the pushbutton pin as an input:
  Serial.println("hi there");
  // initialize the pushbutton pin as an input:
  pinMode(13, OUTPUT);
  
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
  
  startMozzi(CONTROL_RATE);
}


void updateControl(){  
  
  //int knob = mozziAnalogRead(FLT_PIN);
  //byte cutoff_freq = knob>>2;
  // EncoderButton object updates
  eb1.update();
  left.update();
  right.update();
  
  if ( buttonState == 0 ) {
    updateWavePacket();
  } else if ( buttonState == 1 ) {
    updateReso();
  }
  if (buttonState == 1) {
    // turn LED on:
    digitalWrite(13, HIGH);
  } else {
    // turn LED off:
    digitalWrite(13, LOW);
  }
  

}


void updateReso() {

  int note0 = map(kAverageF.next( mozziAnalogRead(FUNDAMENTAL_PIN)), 0, 1023, 24, 72);
  int noteM = map(kAverageM1.next(mozziAnalogRead(A5)), 0, 1023, 16, 32);
  int target_note = note0 - noteM;
  
  x_axis = ( kAverageCf.next(mozziAnalogRead(CENTREFREQ_PIN)) + kAverageM3.next(mozziAnalogRead(A6)) / 2 );
  y_axis = ( kAverageBw.next(mozziAnalogRead(BANDWIDTH_PIN) ) + kAverageM2.next(mozziAnalogRead(A7)) ) / 2;

  //fakeMidiRead(target_note);
  fakeMidiRead(target_note,x_axis);
  voice.update();


}

void fakeMidiRead(int target, int velocity){
  static char curr_note;
  curr_note = target;
  if(startNote.ready()){
    //
    HandleNoteOn(1,curr_note,velocity);
    startNote.set(mozziAnalogRead(A6));
    startNote.start();
    endNote.set(mozziAnalogRead(A7));
    endNote.start();
  }
  if(endNote.ready()){
    HandleNoteOff(1,curr_note,0);
  }
}

void HandleNoteOn(byte channel, byte pitch, byte velocity){
  if (velocity > 0)
  {
    voice.noteOn(channel, pitch, velocity);
    unsigned int attack = kmapX(x_axis);
    unsigned int decay = kmapY(y_axis);
    voice.setPDEnv(attack,decay);
    digitalWrite(13,HIGH);
  }
  else{
    stopNote(channel,  pitch,  velocity);
  }
}
void HandleNoteOff(byte channel, byte pitch, byte velocity){
  stopNote(channel,  pitch,  velocity);
}

void stopNote(byte channel, byte pitch, byte velocity){
  voice.noteOff(channel, pitch, velocity);
  digitalWrite(13,LOW);
}




void updateWavePacket(){
/*  Serial.println("M1");
  Serial.println(mozziAnalogRead(A5));
  Serial.println("M2");
  Serial.println(mozziAnalogRead(A6));
  Serial.println("M3");
  Serial.println(mozziAnalogRead(A7));
 */ 
  wavey.set( 
     (kAverageF.next( mozziAnalogRead(FUNDAMENTAL_PIN)) + kAverageM1.next(mozziAnalogRead(A5) ) / 2)  ,
     kAverageBw.next(mozziAnalogRead(BANDWIDTH_PIN) ) + kAverageM2.next(mozziAnalogRead(A6)) / 2  , 
     kAverageCf.next(2*mozziAnalogRead(CENTREFREQ_PIN)) + kAverageM3.next(mozziAnalogRead(A7)) / 2 
  );
}

AudioOutput_t updateAudio(){
  
  if ( buttonState == 0 ) {
    return wavey.next()>>8;
  } else if ( buttonState == 1 ) {
    return voice.next();
    
  }
   
}



void loop(){
  audioHook(); // required here
}
