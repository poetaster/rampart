#pragma once

#include "nyblybyte.h"

/* 
 *  these methods not yet used. searching an available timer is what we should aim for 
 *  for now, limited to the original on pin 11/timer 2
*/

void PWM16Begin()
{
  // Stop Timer/Counter1
  TCCR1A = 0;  // Timer/Counter1 Control Register A
  TCCR1B = 0;  // Timer/Counter1 Control Register B
  TIMSK1 = 0;   // Timer/Counter1 Interrupt Mask Register
  TIFR1 = 0;   // Timer/Counter1 Interrupt Flag Register
  ICR1 = TOP;
  OCR1A = 0;  // Default to 0% PWM
  OCR1B = 0;  // Default to 0% PWM


  // Set clock prescale to 1 for maximum PWM frequency
  TCCR1B |= (1 << CS10);
  // Set to Timer/Counter1 to Waveform Generation Mode 14: Fast PWM with TOP set by ICR1
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12) ;
}


void PWM16EnableA()
{
  // Enable Fast PWM on Pin 9: Set OC1A at BOTTOM and clear OC1A on OCR1A compare
  TCCR1A |= (1 << COM1A1);
  pinMode(9, OUTPUT);
}


void PWM16EnableB()
{
  // Enable Fast PWM on Pin 10: Set OC1B at BOTTOM and clear OC1B on OCR1B compare
  TCCR1A |= (1 << COM1B1);
  pinMode(10, OUTPUT);
}


inline void PWM16A(unsigned int PWMvalue)
{
  OCR1A = constrain(PWMvalue, 0, TOP);
}


inline void PWM16B(unsigned int PWMvalue)
{
  OCR1B = constrain(PWMvalue, 0, TOP);
}

/* 
 *  I, Mark Washeim do not assert any Copyright for the result formulas. They belong to the public domain
 *  when no other copyright has been asserted.
 *  
 *  Various bytebeat formulas. some from my old midi/arduboy sketches, nyblybyte has a lot of references
 *  https://github.com/erlehmann/algorithmic-symphonies and
 *  https://raw.githubusercontent.com/schollz/nyblcore/main/bytebeat/bytebeat.ino
 */

ISR(TIMER1_COMPA_vect) {

  switch (prog) {
     case 1: 
      // poetaster the next three together
      aMax = 16; aMin = 1; 
      bMax = 16; bMin = 1;
      cMax = 16; cMin = 1;  
      result = t >> c ^ t & 1 | t + (t ^ t >> 13) - t * ((t >> 5 ? b : a) & t >> ( 8 - ( a >> 1 )  ) ); 
      break;
    case 2: // 15, 16 and 17go together.
      aMax = 16; aMin = 1; 
      bMax = 16; bMin = 1;
      cMax = 16; cMin = 1;  
      result =  t >> 4 ^ t & c | t + (t ^ t >> 8) - t * ((t >> 3 ? b : a) & t >> ( 5 - ( b >> 1 )  ) ); 
      // ^ t << 1 & (a & 12 ? t >> 4 : t >> 10)
      break;
    case 3:
      aMax = 16; aMin = 1; 
      bMax = 16; bMin = 1;
      cMax = 16; cMin = 1;  
      result = t >> 4 ^ t & c | t + (t ^ t >> 21) - t * ((t >> 8 ? b : a) & t >> ( 8 - ( b >> 5 )  ) ); 
      break; 
    case 4:
      aMax = 30; aMin = 1; 
      bMax = 15; bMin = 1;
      cMax = 30; cMin = 1;  
      // poetaster the devil. pulses and mana, mana.three controls can cross conflict or harmonize nice. turn c up first
      result = t *  t << 1 & (t & 7 ? t >> 3 : t >> c) ^ ((t >> 7 ? 2 : b) & t >> (c + a)) | t + ((t ^ t >> 13)) | a * t >> b ^ t & c ;
      break;
    case 5:
      // poetaster lead synth a & c mid low, b middle and you have a lead synth drone, it's a neat synth!
      bMax = 45; bMin = 1;
      result = t * ((t >> 7 ? a:c ) & t >> ( a*c)) ^ t << 1 & (t & b ? t >> 5 : t >> c) - b * t >> 3 ^ t & (42 - b) ;
      break;
    case 6:
      // poetaster helicopters has some arps with b in the middle, various!
      result = ( (t >> a) - ( t >> a & t )  + ( t >> t & a) ) + ( t * ((t >> b) & c ) );
      break;
    case 7:
      cMax = 12; cMin = 1;
      aMax = 16; aMin = 1;  
      bMax = 16; bMin = 1;
      // poetaster windy whirls, noisy swirls, ocean swells, drop off.
      result =  t >> ( ( a + 1 )  & b & t >> 8 ) ^ ( t & t >> a | t >>c ); /// maybe t >> 16?
      break;
    case 8:
      // poetaster breaky, jungle stuff a at one oclock, c middle, etc
      cMax = 12; cMin = 1;
      aMax = 16; aMin = 1;  
      bMax = 16; bMin = 1;  
      if (t > 65536) t = -65536; 
      result = a + ( ( t >> a + 1 ) ) * (t >> c | b | t >> ( t >> 16) )  ;
      break;
    case 9:
      // poetaster techno stuff but slow it down and it's rockin
      aMax = 8; aMin = 1; 
      bMax = 10; bMin = 1;
      cMax = 10; cMin = 1;  
      result =  (t & t >> b | t << b >> c) ^ ( t  &  t >> a | t << a >> b) & ( t & t >> c | t << c >> a);
      break;
      
    case 10:
      // poetaster counter point bleeps with a hihat or reverb stuff
      aMax = 30; aMin = 1; 
      bMax = 30; bMin = 1;
      cMax = 30; cMin = 1;  
      result =  ( t  &  t >> c | t + c << 4) | (t & t >> a | t + a << 3) ^ ( t & t >> b | t + b << 2 );

      break;
    case 11:
      // poetaster long distorted carnival number
      aMax = 16; aMin = 1;
      bMax = 16; bMin = 1;
      cMax = 16; cMin = 1;
      result = (t * a & t >> b | t * c & t >> 7 | t * 3 & t / 1024) - 1; 

      break;
    case 12:
      // poetaster twisted calliope distortion can rock :)
      aMax = 30; aMin = 1; 
      bMax = 15; bMin = 1;
      cMax = 15; cMin = 1;  
      result = (  t * b & c / (a << 2) | t * b & t >> c | t * 12 & t >> a) - 1;;
      break;
    case 13:
      // (t*(t>>12)*64+(t>>1)*(t>>10)*(t>>11)*48)>>(((t>>16)|(t>>17))&1
      // http://www.pouet.net/topic.php?which=8357&page=17#c389829",
      // wroom zoom ..... goes long not like th original nice variety

      result = ( t * (t>>c) * 64 + ( t >> 1 ) * ( t >> b ) * (t >>7) * 48 ) >> ( ( (t>>16)|(t>>a) )+1);
      break;
    case 14:
      // poetaster
      // c high, techno wheep, b pitch in the middle, a patterned blips fast to slow
      aMax = 24; aMin = 1;
      bMax = 12;  bMin = 1;
      cMax = 16; cMin = 1;  
      result =  t * ( t >> b & ( t >> b ? 13 : 8)  ) - t >> c ^ t & 21 | t + (t ^ t >> a)  ^ t<<1 & (t & a ? t >> 5 : t >> 3);
      break;    
    case 15:
    // poetaster fast to slow arps + staccato noise and robot voice
      result = (( t >> c | b ) & (a + 1))  * (( t >> (b + 1) ) | t >> ( t >> 21)) ;
      break;
    case 16:
      // poetaster descending / ascending bleep arps, lasers, a high bass run
      result = t - ((t & ((t >> a))) + ( a | t >> c )) & (t >> ( c + 1)) | (t >> b) & (t * (t >> a));
      break;

    case 17:
    // poetaster pulse drone // seems to kill the nano after all?
      aMax = 32; aMin = 1; 
      bMax = 24; bMin = 1;
      cMax = 16; cMin = 1;  
      result = ((t*a) & ( t>>5| t<<2 )  ) | ( (t*b) & ( t>>4 | t<<3)) | ((t*c) & ( t>>3 | t<<4 ) );
      break;
    case 18: 
    // poetaster drone, organ, perc
      aMax = 8; aMin = 1; 
      bMax = 16; bMin = 1;
      cMax = 8; cMin = 1;
      result= ( ( t * a & t >> 4 ) | ( t * b & t >> 7 ) | ( t * c &  t) ) - 1;
      break;
    case 19:
    // poetaster also a drone, basic with perc blurbs
      aMax = 16; aMin = 4; bMax = 16; bMin = 3; cMax = 16; cMin = 1;
      result =  ( t >> a | t - b ) & ( t -a | t >> b ) * c;   
      break;
    case 20:
    // a melodic drone
      aMax = 16; aMin = 8;
      bMax = 14; bMin = 7;
      cMax = 12; cMin = 6;
      result = t - b & ( (t>>a | t<<4 ) ) ^ t - c & ( ( t>>b | t<<3 ) ) ^ t - a & ( ( t>>c | t<<2 ) ) ;
      break;
    case 21:
    // classic vizmut paper pp. 5 https://arxiv.org/pdf/1112.1368.pdf
      aMax = 100; aMin = 20;
      bMax = 50;  bMin = 10;
      cMax = 5;   cMin = 1;
      result = t - b & ( (t>>a | t<<4 ) ) ^ t - c & ( ( t>>b | t<<3 ) ) ^ t - a & ( ( t>>c | t<<2 ) ) ;
      break;
    case 22:
    // classic vizmut paper pp. 6 https://arxiv.org/pdf/1112.1368.pdf
      aMax = 16; aMin = 1;
      bMax = 16; bMin = 1;
      cMax = 16; cMin = 1;
      result = t - b & ( (t>>a | t<<4 ) ) ^ t - c & ( ( t>>b | t<<3 ) ) ^ t - a & ( ( t>>c | t<<2 ) ) ;
      //result = (int)(t/1e7 * t * t + c ) % 127 | t >> c ^ t >> b | t % 127 + ( t >> a ) | t ;
      break;
    case 23:
    // classic vizmut paper pp. 6 https://arxiv.org/pdf/1112.1368.pdf
    // modified to keep from crashing :) divisions replaced with multiplication :)
      aMax = 7; aMin = 1;
      bMax = 7; bMin = 1;
      cMax = 7; cMin = 2;
      result = t >> c | t & (( t >> 5 )/( t >> b*4 - ( t >> a*3 ) & - t >> b*4 - ( t >> a*3 ) ) );
      // t >>4 | t &(( t >> 5 )/( t >> 7 − ( t >> 15 ) & − t >> 7 − ( t >> 15 ) ) )
      break;
    case 24:
    // click mouth harp and hum and other chaos, clicky too :)
      aMax = 15; aMin = 0;
      bMax = 15; bMin = 0;
      cMax = 5; cMin = 0;
      //if (t > 65536) t = -65536;
      result= ((t >> 6 ? 2 : 3) & t * (t >> a) | (a+b+c) - (t >> b)) % (t >> a) + ( a << t | (t >> c) );
      break;
    case 25:
    // clicky burpy
      aMax = 15; aMin = 0;
      bMax = 11; bMin = 0;
      cMax = 9;  cMin = 0;
      result = ( ( (t >> 9 ? a : b) & t * (t >> b) % (t >> c) - (t >> b) ) * (t >> a) ) ;
      break;
    case 26:
      // variation t+(t&1)+(t>>5)*(t>>1)/1|t>>4|t>>8
      //https://dollchan.net/btb/res/3.html#258
      aMax = 37; aMin = 4;
      bMax = 5;  bMin = 1;
      cMax = 12; cMin = 5;
      result = ( t + ( t & b) + ( t >> a ) * ( t >> b ) / 1 | t >> b | t >> c );
      break;
    case 27:
       // variation on https://dollchan.net/btb/res/3.html#78
       // ( ( t >> 10 | t * 5 ) & ( t >> 8 | t * 4 ) & ( t >> 4 | t * 6 ) );
      aMax = 14; aMin = 4;
      bMax = 6;  bMin = 1;
      cMax = 12; cMin = 4;
      result = ( ( t >> a | t * 5 ) & ( t >> ( a + 2 ) | t * b ) & ( t >> b | t * c ) );
      break;
    case 28:
      // https://forum.arduino.cc/t/one-line-algorithmic-music/73409
      // (t*(4|t>>13&3)>>(~t>>11&1)&128|t*(t>>11&t>>13)*(~t>>9&3)&127)^(t&4096?(t*(t^t%255)|t>>4)>>1:t>>3|(t&8192?t<<2:t))
      aMax = 19; aMin = 4;
      bMax = 7;  bMin = 1;
      cMax = 14; cMin = 1;
      result = (t * (4 | t >> 13 & b ) >> ( ~t >> 11 & 1 ) & 128 | t * ( t >> a & t >> 13 ) * ( ~t >> c & 3 ) & 127 ) ^ ( t & 4096 ? ( t * ( t ^ t % 255 ) | t >> 4 ) >> 1 : t >> 3 |( t & 8192 ? t << 2 : t ) );
      break;
 // from here nybly
 // nice 8, 17, ( bb28 | t << c), bb32 << c (or |), 34 great, 37 ( also  << c, | c), b39 ( | t>>c)
 // normal bb0,bb4 bb11 ( bb19 bb21 | t << c) (bb22 + c) bb23 + c, bb33 << c, bb35 << c, 
 // 8192 slow bb5 bb7 bb9 (bb13 | c) bb14 bb16 (bb29 & t>>c), bb30 << c, bb36 | t <<  c;
 
    case 29: 
      aMax = 69; aMin = 1;
      bMax = 69; bMin = 1;
      cMax = 8;  cMin = 0;
      bb39_set(a,b); 
      result =  bb39() ;;
      break;
    case 30: // scratch percussion on th extreme
      cMax = 8; cMin = 0;
      bb37_set(a,b); 
      result =  bb37() | t >> c;
      break;
    case 31:
      bb34_set(a,b); 
      result =  bb34() | t >> c;
      cMax = 8;
      cMin = 0;
      break;
    case 32:
      bb32_set(a,b); 
      result =  bb32() | t >> c;
      cMax = 8;
      cMin = 0;
      break;
    case 33: // not so good at first but it evolves noisy.
      bb28_set(a,b); 
      result =  bb28() |  ~t << c;
      cMax = 8;
      cMin = 0;
      break;
    case 34: // yeah, it's nice and evolves a lot :)
      bb17_set(a,b); 
      result =  bb17() | t >> c;
      cMax = 8;
      cMin = 0;
      break;
    case 35:
      bb8_set(a,b); 
      result =  bb8() | t >> c;
      cMax = 8;
      cMin = 0;
      break;
    case 36:
      bb35_set(a,b); 
      result =  bb35() | t >> c;
      cMax = 8;
      cMin = 0;
      break;
    case 37:
      bb33_set(a,b); 
      result =  bb33() | t >> c;
      cMax = 8;
      cMin = 0;
      break;
    case 38:
      bb23_set(a,b); 
      result =  bb23() | t >> c;
      cMax = 8;
      cMin = 0;
      break;
    case 39:
      aMax = 69;
      aMin = 1;
      bMax = 69;
      bMin = 1;
      cMax = 8;
      cMin = 0;
      bb22_set(a,b); 
      result =  bb22() | t >> c;
      break;
    case 40:
      // Nightmachines https://forum.aemodular.com/post/235
      aMax = 16;
      aMin = 1;
      bMax = 16;
      bMin = 1;
      cMax = 16;
      cMin = 1;
      result  = (t * 4 | t | t >> 3 & t + t / 4 & t * a | t * 8 >> b | t / c & t + 140) & t >> 4;
      break;
    case 41:
      //Street Surfer by skurk, raer (2011-09-30) https://www.pouet.net/topic.php?which=8357&page=4#c388479
      aMax = 16;
      aMin = 1;
      bMax = 16;
      bMin = 1;
      cMax = 16;
      cMin = 1;
      result  = t & (4096) ? t / 2 * (t ^ t % (a << 1)) | t >> 5 : t / (a << 1) | (t & (b << 7) ? 4 * t : t);
      break;
    case 42:
      // from http://xifeng.weebly.com/bytebeats.html we have someting similar already
      aMax = 16;
      aMin = 1;
      bMax = 16;
      bMin = 1;
      cMax = 16;
      cMin = 1;
      result  = ((((((((t >> a) | t) | (t >> a)) * c) & ((5 * t) | (t >> c))) | (t ^ (t % b))) & 0xFF));
      break;
    case 43:
      // Extraordinary thread of FRACTAL MUSIC by Anonymous from russian imageboards (2014-07-12) http://arhivach.ng/thread/28592/#71678984
      aMax = 96;
      aMin = 24;
      bMax = 8;
      bMin = 1;
      cMax = 16;
      cMin = 1;            
      result = t >> b + t % a | t >> c + t % (t / 31108 & 1 ? 46 : 43) | t / b | t / c >> a; // % a crashes
      break;
    case 44:
      // xpansive 2011-09-29 https://www.pouet.net/topic.php?which=8357&page=3#c388375
      // t * (t >> 8 | t >> 9) & 46 & t >> 8 ^ (t & t >> 13 | t >> 6);
      aMax = 56;
      aMin = 40;
      bMax = 9;
      bMin = 1;
      cMax = 16;
      cMin = 1;  
      result = t * (t >> 8 | t >> 9) & a & t >> 8 ^ (t & t >> c | t >> b);
      break;
    case 45: // straight rythmic, great range!
      aMax = 16;
      aMin = 1;
      bMax = 8;
      bMin = 1;
      cMax = 12;
      cMin = 1;  
      // tejeez 2011-10-05 #countercomplex
      result = (~t >> 2) * ((127 & t * (b & t >> 10)) < (245 & t * (2 + (c & t >> a))));
      break;
      
    
  }

  //PWM16B(result);
  OCR2A =  result;
  if ( enc_offset != 0) t += enc_offset;

  
}




void pwmSetup()
{
  
  ASSR &= ~(_BV(EXCLK) | _BV(AS2));
  
  TCCR2A |= _BV(WGM21) | _BV(WGM20);
  TCCR2B &= ~_BV(WGM22);

  // Do non-inverting PWM on pin OC2A (p.155), p11
  TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
  TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));
  // No prescaler (p.158)
  TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);
  
  // Set initial pulse width to the first sample.
  OCR2A = 0;
  // Set up Timer 1 to send a sample every interrupt.
  cli();

  // Set CTC mode (Clear Timer on Compare Match) (p.133)
  // Have to set OCR1A *after*, otherwise it gets reset to 0!
  TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12 );
  TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10) );

  // No prescaler (p.134)
  TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

  // Set the compare register (OCR1A).
  // OCR1A is a 16-bit register, so we have to do this with
  // interrupts disabled to be safe.
  OCR1A = F_CPU / SRATE;    // 16e6 / 8000 = 2000
  // Enable interrupt when TCNT1 == OCR1A (p.136)
  TIMSK1 |= _BV(OCIE1A);
  sei();
  
}



/* Sound definitions : these the the old methods from my midiboy version here for reverence
byte sound1(int i) {
  return (i / 13 >> (1 + ((i >> 12) & 3)) | i / 2 >> 2 & (i / 6) >> 7 | i & 31 * i * (i >> 8));
}
byte sound2(int i) {
  return ((i / 3 >> (i % 40 + 5) | i / (24 + i & 3) >> (i % (15 - ((i >> 15) % 8) * 6) + 5)) / 8);
}

byte sound3(int i) {  
  return   ( ( (i * 15) & (i >> 5)) | ((i *2) & (i>>7)) | ((i*8)&(i>>11)) * (i >> 8) );
}

byte sound4(int i) {
  return int( ((sin(i * 0.005) / 2.0) + 1) * 127);
}
byte sound5(int i) {
  return (i * ((i >> 12 | i >> 8) & 63 & i >> 4));
}
byte sound6(int i) {
  return ((i ^ i >> 8) | i << 3 & 56 ^ i);
}
byte sound7(int i) {
  return ( ((((i>>12)^(i>>12)-2)%11*i)/4|i>>13)&127);
  //return i * 4 / 64 | i & i / 128;
}
byte sound8(int i) {
  return (((i << 1) ^ ((i << 1) + (i >> 7)&i >> 12)) | i >> (4 - (1 ^ 7 & (i >> 19))) | i >> 7);
}
byte sound9(int i) {
  return int ((i * (i >> 8 + i >> 9) * 100) + sin(i));
  //return int(i & (i << 3) % (72022 & (i >> 9)) * 283742783 & i);
}
byte sound10(int i) {
  return  (i * ((i >> 5 | i >> 8) >> (i >> 16)));
}
byte sound11(int i) {
  return (((i * (i >> 8 | i >> 9) & 46 & i >> 8)) ^ (i & i >> 13 | i >> 6));
}
byte sound12(int i) {
 return  ((((i >> 12) ^ ((i >> 10)+ 3561)) %5) * i & 128 | ((((i >> 11) ^ ((i >> 9) +2137)) %5) * i) & ((i>>14)%120+8) | (i>>5));
 //return i * ((i >> 16 | i >> 12) & 215 & ~i >> 8);
}

byte getSound(byte soundtype, int i) {
  //yes yes, can be done with function array pointers.
  switch (soundtype) {
    case 1:
      {
        return  sound1(i);
      }
    case 2:
      {
        return  sound2(i);
      }
    case 3:
      {
        return  sound3(i);
      }
    case 4:
      {
        return  sound4(i);
      }
    case 5:
      {
        return  sound5(i);
      }
    case 6:
      {
        return  sound6(i);
      }
    case 7:
      {
        return  sound7(i);
      }
    case 8:
      {
        return  sound8(i);
      }
    case 9:
      {
        return  sound9(i);
      }
    case 10:
      {
        return  sound10(i);
      }
    case 11: {
        return sound11(i);
      }
    case 12: {
        return sound12(i);
      }
  }
}
*/
