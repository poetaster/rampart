/*  Plays a fluctuating ambient wash using pairs
    of slightly detuned oscillators, following an example
    from Miller Puckette's Pure Data manual.
  
    The detune frequencies are modified by chance in
    updateControl(), and the outputs of 14 audio
    oscillators are summed in updateAudio().
  
    Demonstrates the use of fixed-point Q16n16
    format numbers, mtof() for converting midi note
    values to frequency, and xorshift96() for random numbers.
  
    Circuit: Audio output on digital pin 9 on a Uno or similar, or
    DAC/A14 on Teensy 3.0/3.1, or 
    check the README or http://sensorium.github.com/Mozzi/
  
    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users
  
    Tim Barrass 2012, CC by-nc-sa.
*/

//#include <ADC.h>  // Teensy 3.0/3.1 uncomment this line and install http://github.com/pedvide/ADC
#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/cos8192_int8.h>
#include <mozzi_rand.h>
#include <mozzi_midi.h>
#include <mozzi_fixmath.h> // for Q16n16 fixed-point fractional number type

// harmonics
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos1(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos2(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos3(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos4(COS8192_DATA);


// duplicates but slightly off frequency for adding to originals
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos1b(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos2b(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos3b(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos4b(COS8192_DATA);


// base pitch frequencies in 24n8 fixed int format (for speed later)
Q16n16 f1,f2,f3,f4,f5,f6,f7;


Q16n16 variation() {
  // 32 random bits & with 524287 (b111 1111 1111 1111 1111)
  // gives between 0-8 in Q16n16 format
  return  (Q16n16) ( ( map(mozziAnalogRead(A4),1,1023,100,50000)+xorshift96() /2) & 524287UL); //xorshift96() & 524287UL);
}


void setup(){
  startMozzi(64); // a literal control rate here
  Serial.begin(9600);
  // select base frequencies using mtof (midi to freq) and fixed-point numbers
  f1 = Q16n16_mtof(Q16n0_to_Q16n16(48));
  f2 = Q16n16_mtof(Q16n0_to_Q16n16(74));
  f3 = Q16n16_mtof(Q16n0_to_Q16n16(64));
  f4 = Q16n16_mtof(Q16n0_to_Q16n16(77));


  // set Oscils with chosen frequencies
  aCos1.setFreq_Q16n16(f1);
  aCos2.setFreq_Q16n16(f2);
  aCos3.setFreq_Q16n16(f3);
  aCos4.setFreq_Q16n16(f4);


  // set frequencies of duplicate oscillators
  aCos1b.setFreq_Q16n16(f1+variation());
  aCos2b.setFreq_Q16n16(f2+variation());
  aCos3b.setFreq_Q16n16(f3+variation());
  aCos4b.setFreq_Q16n16(f4+variation());

}


void loop(){
  audioHook();
}


void updateControl(){
  // todo: choose a nice scale or progression and make a table for it
  // or add a very slow gliss for f1-f7, like shephard tones

  int trig = map(mozziAnalogRead(A4), 0, 1023, 1, 100);
  int offset = map(mozziAnalogRead(A3), 0, 1023, 25, 250);
    
  f1 = Q16n16_mtof(Q16n0_to_Q16n16(map(mozziAnalogRead(A0), 0, 1023, 24, 64)));
  f2 = Q16n16_mtof(Q16n0_to_Q16n16(map(mozziAnalogRead(A1), 0, 1023, 36, 74)));
  f3 = Q16n16_mtof(Q16n0_to_Q16n16(map(mozziAnalogRead(A2), 0, 1023, 24, 77)));
  f4 = Q16n16_mtof(Q16n0_to_Q16n16(map(mozziAnalogRead(A3), 0, 1023, 48, 86)));
  // ok, now modulate on the pins ! :)
  f1 = Q16n16_mtof(Q16n0_to_Q16n16(map(mozziAnalogRead(A4), 0, 1023, f1, 64)));
  f2 = Q16n16_mtof(Q16n0_to_Q16n16(map(mozziAnalogRead(A5), 0, 1023, f2, 74)));
  f3 = Q16n16_mtof(Q16n0_to_Q16n16(map(mozziAnalogRead(A6), 0, 1023, f3, 77)));
  f4 = Q16n16_mtof(Q16n0_to_Q16n16(map(mozziAnalogRead(A7), 0, 1023, f4, 86)));
  Serial.println(variation());
  // change frequencies of the b oscillators
  switch (lowByte(xorshift96()) & 7){ // 7 is 0111

    case 0:
      aCos1b.setFreq_Q16n16(f1+variation());
    break;

    case 1:
       aCos2b.setFreq_Q16n16(f2+variation());
    break;

    case 2:
       aCos3b.setFreq_Q16n16(f3+variation());
    break;

    case 3:
       aCos4b.setFreq_Q16n16(f4+variation());
    break;

  }
}


int updateAudio(){

  int asig =
    aCos1.next() + aCos1b.next() +
    aCos2.next() + aCos2b.next() +
    aCos3.next() + aCos3b.next() +
    aCos4.next() + aCos4b.next();

  return asig >> 3;
}
