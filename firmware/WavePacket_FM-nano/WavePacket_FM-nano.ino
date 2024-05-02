/*   This program is based on the Mozzi examples WavePacket_Double and FMsynth
     Mark Washeim <blueprint@poetaster.de) 2020, public domain.

     Tim Barrass 2013, CC by-nc-sa.
*/

//#include <ADC.h>  // Teensy 3.0/3.1 uncomment this line and install http://github.com/pedvide/ADC

#include <Mozzi.h>
#include <mozzi_analog.h>
#include <WavePacket.h>
#include <Ead.h>
#include <RollingAverage.h>
// for FMsynth
#include <Oscil.h>
#include <tables/cos2048_int8.h> // table for Oscils to play
#include <mozzi_midi.h>
#include <mozzi_rand.h>
#include <mozzi_fixmath.h>
#include <EventDelay.h>
#include <Smooth.h>
#include <AutoMap.h> // maps unpredictable inputs to a range
#include <ADSR.h>

//#define MOZZI_AUDIO_RATE 32768
//#define MOZZI_PWM_RATE 32768

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

// Map Analogue channels
#define SYNC_CONTROL         (4)

#define CONTROL_RATE 256 // powers of 2 please

// for smoothing the control signals
// use: RollingAverage <number_type, how_many_to_average> myThing
RollingAverage <int, 8> kAverageF;
RollingAverage <int, 8> kAverageBw;
RollingAverage <int, 8> kAverageCf;
RollingAverage <int, 8> kAverageM1;
RollingAverage <int, 8> kAverageM2;
RollingAverage <int, 8> kAverageM3;
WavePacket <SINGLE> wavey; // <DOUBLE> wavey; // DOUBLE selects 2 overlapping streams

ADSR <CONTROL_RATE, AUDIO_RATE> envelope;
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
int gain;

// for note changes
Q7n8 last_note, target_note, note0, note1, note_upper_limit, note_lower_limit, note_change_step, smoothed_note;

// Inefficient, but...until there is a better Smooth....
Smooth <int> kSmoothNote(0.95f);


// variables will change: 1-3
int buttonState = 0;

#include "encoder.h"

void setup() {

  Serial.begin(9600);
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
  
  // FMsetup
  kNoteChangeDelay.set(768); // ms countdown, taylored to resolution of CONTROL_RATE
  kModIndex.setFreq(.768f); // sync with kNoteChangeDelay
  target_note = note0;
  last_note = target_note;
  note_change_step = Q7n0_to_Q7n8(3);
  note_upper_limit = Q7n0_to_Q7n8(64);
  note_lower_limit = Q7n0_to_Q7n8(24);
  note0 = note_lower_limit;
  note1 = note_lower_limit + Q7n0_to_Q7n8(5);
  attack = 0;
  decay = 500;
  
  envelope.setADLevels(255, 200);
  envelope.setTimes(50, 200, 1000, 200); // 10000 is so the note will sustain 10 seconds unless a noteOff comes
  
  // for the env
  randSeed(); // fresh random
  startMozzi(CONTROL_RATE);
}


void updateControl() {
  
  // EncoderButton object updates
  eb1.update();
  left.update();
  right.update();

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


  //byte cutoff_freq = knob>>4;
  //kAverageF.next( mozziAnalogRead(FUNDAMENTAL_PIN)>>1 ) + kAverageM1.next(mozziAnalogRead(A5)>>1 ) / 2  ,
  note0 = map(mozziAnalogRead(FUNDAMENTAL_PIN), 0, 1023,1024, 6144);
  note1 = map(mozziAnalogRead(M1P), 0, 1023, 1024, 6144);
  
  if (note1 > 10) {
    target_note = note0 + note1 / 2;
  } else {
    target_note = note0;
  }
/*
  if (kNoteChangeDelay.ready()) {
    //envelope.start(attack, decay);
    // reset eventdelay
    
    kNoteChangeDelay.start(256);
    envelope.noteOn();
  } else{
    //envelope.noteOff();
  }
*/
  //Serial.println(mozziAnalogRead(CENTREFREQ_PIN));
  //Serial.println('-');
  //Serial.println(mozziAnalogRead(BANDWIDTH_PIN));
  //envelope.update();
  //gain = (int) envelope.next();

  last_note = target_note;
  //Serial.println(target_note);
  int modulate, modI;
  int bw = mozziAnalogRead(BANDWIDTH_PIN) ;
  int mw = mozziAnalogRead(M2P);
  if ( mw > 10 ) {
    modulate = ( bw + mw  ) / 2;
    modI = map(modulate, 0, 1023, 256, 768);
  } else {
    modI = map(bw, 0, 1023, 256, 768);
  }
  
  mod_to_carrier_ratio = (Q8n8)(map( mozziAnalogRead(CENTREFREQ_PIN), 0, 1023, 1,8));
  // vary the modulation index
  mod_index = (Q8n8)modI + kModIndex.next();

  smoothed_note = kSmoothNote.next(target_note);
  setFreqs(target_note);


}

//FM
void setFreqs(Q8n8 midi_note) {

  carrier_freq = Q16n16_mtof(Q8n8_to_Q16n16(midi_note)); // convert midi note to fractional frequency
  mod_freq = ((carrier_freq>>8) * mod_to_carrier_ratio)  ; // (Q16n16>>8)   Q8n8 = Q16n16, beware of overflow
  deviation = ((mod_freq>>16) * mod_index); // (Q16n16>>16)   Q8n8 = Q24n8, beware of overflow
  aCarrier.setFreq_Q16n16(carrier_freq);
  aModulator.setFreq_Q16n16(mod_freq);

}

void updateWavePacket() {
  
  wavey.set(
    (map( (mozziAnalogRead(FUNDAMENTAL_PIN) - mozziAnalogRead(M1P)),  0, 1023, 8, 254) ),
    (map( (mozziAnalogRead(BANDWIDTH_PIN)  - mozziAnalogRead(M2P)),  0, 1023, 128, 1023) ),
    (mozziAnalogRead(CENTREFREQ_PIN) + mozziAnalogRead(M3P)/ 2)
  );
}

AudioOutput updateAudio() {

  if ( buttonState == 0 ) {
    return wavey.next()  >> 8;
  } else if ( buttonState == 1 ) {
    Q15n16 modulation = deviation * aModulator.next() >> 8;
    return MonoOutput::from8Bit(aCarrier.phMod(modulation)); // envelope.next()
  }
  return 0; // should not get here
}

void loop() {
  audioHook(); // required here
}
