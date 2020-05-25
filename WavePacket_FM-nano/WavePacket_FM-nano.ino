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
 
//#include <ADC.h>  // Teensy 3.0/3.1 uncomment this line and install http://github.com/pedvide/ADC
#include <MozziGuts.h>
#include <mozzi_analog.h>
#include <WavePacket.h>
#include <ADSR.h>
#include <RollingAverage.h>
// for FMsynth
#include <Oscil.h>
#include <tables/cos2048_int8.h> // table for Oscils to play
#include <mozzi_midi.h>
#include <mozzi_rand.h>
#include <mozzi_fixmath.h>
#include <EventDelay.h>
#include <Smooth.h>

// analog freq pins
#define FUNDAMENTAL_PIN 0
#define BANDWIDTH_PIN 1
#define CENTREFREQ_PIN 2

// modulation pins analog
#define M1_PIN 5
#define M1_PIN 6
#define M1_PIN 7
#define FLT_PIN 3

// button pins digita
#define B2_PIN 2
#define B3_PIN 3

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

ADSR <CONTROL_RATE, AUDIO_RATE> envelope;
//byte attack_level = rand(128)+127;
//byte decay_level = rand(255);
unsigned int duration, attack, decay, sustain, release_ms;

// for FMsynth

Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aCarrier(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aModulator(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, CONTROL_RATE> kModIndex(COS2048_DATA);

Q8n8 mod_index;// = float_to_Q8n8(2.0f); // constant version
Q16n16 deviation;
Q16n16 carrier_freq, mod_freq;

// FM ratio between oscillator frequencies, stays the same through note range
Q8n8 mod_to_carrier_ratio = float_to_Q8n8(3.f);

EventDelay kNoteChangeDelay;

// for note changes
Q7n8 last_note, target_note, note0, note1, note_upper_limit, note_lower_limit, note_change_step, smoothed_note;

// Inefficient, but...until there is a better Smooth....
Smooth <int> kSmoothNote(0.95f);


// variables will change: 1-3
int buttonState = 0; 


void setup(){
  
  //Serial.begin(115200);
  // initialize the pushbutton pin as an input:
  pinMode(B2_PIN, INPUT);
  pinMode(B3_PIN, INPUT);
  pinMode(13, OUTPUT);
  
// FMsetup
  kNoteChangeDelay.set(768); // ms countdown, taylored to resolution of CONTROL_RATE
  kModIndex.setFreq(.768f); // sync with kNoteChangeDelay
  target_note = note0;
  last_note = target_note;
  note_change_step = Q7n0_to_Q7n8(3);
  note_upper_limit = Q7n0_to_Q7n8(72);
  note_lower_limit = Q7n0_to_Q7n8(24);
  note0 = note_lower_limit;
  note1 = note_lower_limit + Q7n0_to_Q7n8(5);
  // for the env
  randSeed(); // fresh random 
  startMozzi(CONTROL_RATE);
}


void updateControl(){  

  static int previous2;
  int current2 = digitalRead(B2_PIN);
  if (previous2 == LOW && current2 == HIGH) {
    if(buttonState == 1) {
       buttonState = 0;
    } else {
      buttonState = 1;
    }
  }
  previous2 = current2; 
  if ( buttonState == 0 ) {
    updateWavePacket();   
  } else if ( buttonState == 1 ) {
    updateFM();
  }
   if (buttonState == 1) {
    // turn LED on:
    digitalWrite(13, HIGH);
  } else {
    // turn LED off:
    digitalWrite(13, LOW);
  }
  
}

void updateFM() {

  int knob = map(mozziAnalogRead(FLT_PIN),0,1023,8,512);
  byte cutoff_freq = knob>>4;
  //kAverageF.next( mozziAnalogRead(FUNDAMENTAL_PIN)>>1 ) + kAverageM1.next(mozziAnalogRead(A5)>>1 ) / 2  ,
  int note0 = map(mozziAnalogRead(FUNDAMENTAL_PIN), 0, 1023, 200, 10000);
  int noteM = map(mozziAnalogRead(A5), 0, 1023, 100, 10000);
  int target_note = note0 + noteM /2;
  
  
  
  if(kNoteChangeDelay.ready()){

    
    // choose envelope levels
    byte attack_level = knob+256;
    byte decay_level = knob+127;
    envelope.setADLevels(attack_level,decay_level);
    unsigned int new_value = knob;
       attack = new_value;
       decay = attack + 64; //map(new_value,1,255,64,256) ;
       sustain = decay; //map(new_value,1,255,64,256);
       release_ms = attack;

     envelope.setTimes(attack,decay,sustain,release_ms);
          
      //envelope.noteOff();
      envelope.noteOn();


    // reset eventdelay
    kNoteChangeDelay.start(attack+decay+sustain+release_ms);
  }

  envelope.update();
  last_note = target_note;
  //Serial.println(target_note);
  int modulate = ( mozziAnalogRead(BANDWIDTH_PIN)  + mozziAnalogRead(A6) ) / 2;
  int modI = map(modulate, 0,1023,200,2800);
  
  // vary the modulation index
  mod_index = (Q8n8)modI+kModIndex.next();
  
  //smoothed_note = kSmoothNote.next(target_note);
  setFreqs(target_note);
  
  
}

//FM
void setFreqs(Q8n8 midi_note){

  carrier_freq = Q16n16_mtof(Q8n8_to_Q16n16(midi_note)); // convert midi note to fractional frequency  
  //int dev = map( kAverageM3.next(mozziAnalogRead(A7)), 0,1023,0,mod_index);
  int dev = map( ( mozziAnalogRead(CENTREFREQ_PIN) + mozziAnalogRead(A7) / 2 ), 0,1023,0,mod_index);  
  mod_freq = ((carrier_freq>>8) * mod_to_carrier_ratio)  ; // (Q16n16>>8)   Q8n8 = Q16n16, beware of overflow  
  deviation = ((mod_freq>>16) * dev); // (Q16n16>>16)   Q8n8 = Q24n8, beware of overflow was * mod_index  
  aCarrier.setFreq_Q16n16(carrier_freq);
  aModulator.setFreq_Q16n16(mod_freq);

}

void updateWavePacket(){
  wavey.set( 
     (mozziAnalogRead(FUNDAMENTAL_PIN) + mozziAnalogRead(A5)  / 2) +1   ,
     (mozziAnalogRead(BANDWIDTH_PIN)  + mozziAnalogRead(A6) /2 )  , 
     (2*mozziAnalogRead(CENTREFREQ_PIN) + mozziAnalogRead(A7)>>1 / 2)
  );
}

int updateAudio(){
  
  if ( buttonState == 0 ) {
    return wavey.next()  >> 8;
  } else if ( buttonState == 1 ) {
    Q15n16 modulation = deviation * aModulator.next() >> 8;
    return ((int)envelope.next()*(int)aCarrier.phMod(modulation))>>8;   
  }
   
}

void loop(){
  audioHook(); // required here
}
