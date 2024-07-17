/*   This program is based on the Mozzi examples WavePacket_Double and FMsynth
     Mark Washeim <blueprint@poetaster.de) 2020, public domain.

     Tim Barrass 2013, CC by-nc-sa.
*/

//#include <ADC.h>  // Teensy 3.0/3.1 uncomment this line and install http://github.com/pedvide/ADC
#include <MozziConfigValues.h>
//#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_2PIN_PWM
#define MOZZI_ANALOG_READ_RESOLUTION 10

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

bool debug = true;

// analog freq pins
#define FUNDAMENTAL_PIN 0
#define BANDWIDTH_PIN 1
#define CENTREFREQ_PIN 2

// modulation pins analog
#define M1P 3
#define M2P 6
#define M3P 7
#define FLT_PIN 5

// button pins digita
#define B2_PIN 2
#define B3_PIN 3

// Map Analogue channels
#define SYNC_CONTROL         (4)
#define CONTROL_RATE 256 // powers of 2 please



// from PDresonant
#include <IntMap.h>
#include "PDResonant.h"
// wavetable for oscillator:
#include <tables/sin2048_int8.h>
PDResonant voice;
// Reso! analog joystick for controlling speed of modulation: assigned to attack, decay times and sustain level
#define X A0
#define Y A1
unsigned int x_axis = 512; // give a static value for test without midi
unsigned int y_axis = 512;
const IntMap kmapX(0, 1023, 0, 500); // A
const IntMap kmapY(0, 1023, 0, 1000); //D

// Intmap is a pre-calculated faster version of Arduino's map, OK for pots
IntMap kMapF(0,1023,18,84);

// AutoMap adapts to range of input as it arrives, useful for LDR's
//AutoMap kMapBw(0,1023,MIN_BW,MAX_BW);
//AutoMap kMapCf(0,1023,MIN_CF,MAX_CF);

// for fake midi
EventDelay startNote; 
EventDelay endNote;

// for smoothing the control signals
// use: RollingAverage <number_type, how_many_to_average> myThing
RollingAverage <int, 24> kAverageF;
//RollingAverage <int, 20> kAverageBw;
//RollingAverage <int, 20> kAverageCf;

RollingAverage <int, 10> kAverageM1;
RollingAverage <int, 10> kAverageM2;
RollingAverage <int, 10> kAverageM3;

WavePacket <SINGLE> wavey; // <DOUBLE> wavey; // DOUBLE selects 2 overlapping streams

ADSR <CONTROL_RATE, AUDIO_RATE> envelope;
unsigned int duration, attack, decay, sustain, release_ms;

// for FMsynth

Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aCarrier(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aModulator(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, CONTROL_RATE> kModIndex(COS2048_DATA);

Q8n8 mod_index;// = float_to_Q8n8(2.0f); // constant version
Q16n16 deviation;
Q16n16 carrier_freq, mod_freq, fundemental, bandwidth, centre;

// FM ratio between oscillator frequencies, stays the same through note range
Q8n8 mod_to_carrier_ratio = float_to_Q8n8(3.f);

EventDelay kNoteChangeDelay;
int gain;

// for note changes
Q7n8 last_note, note_upper_limit, note_lower_limit, note_change_step, smoothed_note;

// Inefficient, but...until there is a better Smooth....
Smooth <int> kSmoothNote(0.95f);


// variables will change: 1-3
int buttonState = 0;

#include "encoder.h"

void setup() {

  Serial.begin(57600);
  Serial.println("hi there");
  // initialize the pushbutton pin as an input:
  pinMode(13, OUTPUT);
  
  analogReference(INTERNAL4V096);
  analogReadResolution(10); // Resolution = 10, 11 or 12 Bit
  
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
  note_change_step = Q7n0_to_Q7n8(3);
  note_upper_limit = Q7n0_to_Q7n8(64);
  note_lower_limit = Q7n0_to_Q7n8(24);
  //note0 = note_lower_limit;
  //note1 = note_lower_limit + Q7n0_to_Q7n8(5);
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
    updateReso();
  } else if ( buttonState == 1 ) {
    updateFM();
  } else if ( buttonState == 2 ) {
    updateWavePacket();
  }
  /*if (buttonState == 1) {
    // turn LED on:
    digitalWrite(13, HIGH);
  } else {
    // turn LED off:
    digitalWrite(13, LOW);
  }*/

}

void updateFM() {

  //byte cutoff_freq = knob>>4;
  //kAverageF.next( mozziAnalogRead(FUNDAMENTAL_PIN)>>1 ) + kAverageM1.next(mozziAnalogRead(A5)>>1 ) / 2  ,
  int note0 = map(mozziAnalogRead<10>(FUNDAMENTAL_PIN), 0, 1023, 1024, 6144);
  int note1;
  int target_note;
  int nw = mozziAnalogRead<10>(M1P);
  if (nw > 200) { 
    note1 = map( nw, 0, 1023, 1024, 6144); 
  } else {
    note1 = 0;
  }

  // make sure we only mix if we have a signal on mod pin
  if (note1 > 1024) {
    target_note = note0 + note1 / 2;
  } else {
    target_note = note0;
  }
  
  //Serial.println(mozziAnalogRead(CENTREFREQ_PIN));
  //Serial.println('-');
  //Serial.println(mozziAnalogRead(BANDWIDTH_PIN));
  //envelope.update();
  //gain = (int) envelope.next();

  last_note = target_note;
  //Serial.println(target_note);
  int modulate, modI;
  int bw = mozziAnalogRead<10>(BANDWIDTH_PIN) ;
  int mw = mozziAnalogRead<10>(M2P);
  
  // make sure we only mix if we have a signal on mod pin
  if ( mw > 50 ) {
    modulate = ( bw + mw  ) / 2;
    modI = map(modulate, 0, 1023, 256, 768);
  } else {
    modI = map(bw, 0, 1023, 256, 768);
  }
  
  int cw = mozziAnalogRead<10>(CENTREFREQ_PIN) ;
  int cm = mozziAnalogRead<10>(M3P);
    // make sure we only mix if we have a signal on mod pin
  if ( cm > 10 ) {
    centre = map( (( cw + cm  ) / 2 ), 0, 1023, 1, 10);
  } else {
    centre = map(cw, 0, 1023, 1, 10);
  }
  mod_to_carrier_ratio = centre;
  //mod_to_carrier_ratio = (Q8n8)(map( mozziAnalogRead(CENTREFREQ_PIN), 0, 1023, 1,8));
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

/*
    wavey.set(kAverageF.next(mozziAnalogRead<10>(FUNDAMENTAL_PIN))+10, // 10 - 1024
    kAverageBw.next(mozziAnalogRead<10>(BANDWIDTH_PIN)), // (0 -1023)
    kAverageCf.next(mozziAnalogRead<11>(CENTREFREQ_PIN))); // 0 - 2047
  */
  int note0 = map(mozziAnalogRead<10>(FUNDAMENTAL_PIN), 0, 1023, 8, 254);
  int note1 = map(mozziAnalogRead<10>(M1P), 0, 1023, 8, 254);
  int target_note;
  // make sure we only mix if we have a signal on mod pin
  if (note1 > 10) {
    target_note = note0 + note1 / 2;
  } else {
    target_note = note0;
  }
  target_note = note0;
  int bw = mozziAnalogRead<10>(BANDWIDTH_PIN) ;
  int bm = mozziAnalogRead<10>(M2P);
    // make sure we only mix if we have a signal on mod pin
  if ( bm > 133 ) {
    bandwidth = map( (( bw + bm  ) / 2 ), 0, 1023, 128, 1023);
  } else {
    bandwidth = map(bw, 0, 1023, 128, 1023);
  }
  
  int cw = mozziAnalogRead<10>(CENTREFREQ_PIN) ;
  int cm = mozziAnalogRead<10>(M3P);
    // make sure we only mix if we have a signal on mod pin
  if ( cm > 10 ) {
    centre = map( (( cw + cm  ) / 2 ), 0, 1023, 8, 512);
  } else {
    centre = map(cw, 0, 1023, 8, 512);
  }
  
  wavey.set( target_note, bandwidth, centre );
  //wavey.set( note0,  map(bw, 0, 1023, 128, 1023), map(cw, 0, 1023, 8, 512) );
 
  
}

void updateReso() {

  int noteA = map(mozziAnalogRead<7>(FUNDAMENTAL_PIN), 0, 512, 16, 84);  
  int noteB = kMapF(mozziAnalogRead<7>(M1P)); 
  int target_note;

  target_note = noteA;
  
  if (noteB > noteA) {
    target_note = noteB;
  } else {
    target_note = noteA;
  }
  
  x_axis = ( kmapX(mozziAnalogRead<10>(CENTREFREQ_PIN)) ) ; // + kAverageM3.next(mozziAnalogRead<10>(A6)) / 2 );
  y_axis = ( kmapY(mozziAnalogRead<10>(BANDWIDTH_PIN) ) ) ; // + kAverageM2.next(mozziAnalogRead<10>(A7)) ) / 2;

    if (debug == true) {
    Serial.print(noteA);
    Serial.print("  ");
    Serial.println(noteB);
    Serial.print(x_axis);
    Serial.print("  ");
    Serial.println(y_axis);
  }
  
  /*
  int bw = mozziAnalogRead(BANDWIDTH_PIN) ;
  int bm = mozziAnalogRead(M2P);
    // make sure we only mix if we have a signal on mod pin
  if ( bm > 133 ) {
    y_axis = kAverageCf.next( ( ( bw + bm  ) / 2 ) );
  } else {
    y_axis = kAverageCf.next(bw);
  }
  
  int cw = mozziAnalogRead(CENTREFREQ_PIN) ;
  int cm = mozziAnalogRead(M3P);
    // make sure we only mix if we have a signal on mod pin
  if ( cm > 10 ) {
    x_axis = kAverageBw.next( ( ( cw + cm  ) / 2  ) );
  } else {
    x_axis = kAverageBw.next(cw);
  }
  */
  //fakeMidiRead();
  
  fakeMidiRead(target_note,x_axis);
  voice.update();
}

AudioOutput updateAudio() {

  if ( buttonState == 0 ) {
    return  MonoOutput::from8Bit( voice.next() )  ;
  } else if ( buttonState == 1 ) {
    Q15n16 modulation = deviation * aModulator.next() >> 8;
    return MonoOutput::from8Bit(aCarrier.phMod(modulation)); // envelope.next()
  } else if ( buttonState == 2 ) {
     return  MonoOutput::from16Bit( wavey.next() )  ;
  }
  return 0; // should not get here
}

void loop() {
  audioHook(); // required here
}

void fakeMidiRead(int target, int velocity){
  static char curr_note;
  curr_note = target;
  if(startNote.ready()){
    //
    HandleNoteOn(1,curr_note,velocity);
    startNote.set(kmapX(x_axis)*2); // startNote.set(mozziAnalogRead(A6));
    startNote.start();
    endNote.set(kmapY(y_axis)*2); //endNote.set(mozziAnalogRead(A7));
    endNote.start();
  }
  if(endNote.ready()){
    HandleNoteOff(1,curr_note,0);
  }
}
/*
void fakeMidiRead(){
  static char curr_note;
  if(startNote.ready()){
    curr_note = 20+rand(40);
    HandleNoteOn(1,curr_note,127);
    startNote.set(2000);
    startNote.start();
    endNote.set(1000);
    endNote.start();
  }
  if(endNote.ready()){
    HandleNoteOff(1,curr_note,0);
  }
}*/
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
