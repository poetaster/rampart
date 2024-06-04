#pragma once

#include "nyblybyte.h"

/* CTC PWM on pin 9 
 * to keep it consistant with mozzi  
 * sadly, seems to collide with the encoder button library?
 */
   
void pwmSetupPin9(){

  ASSR &= ~(_BV(EXCLK) | _BV(AS2));
  TCCR1A |= _BV(WGM21) | _BV(WGM20);
  TCCR1B &= ~_BV(WGM22);
  TCCR1A = (TCCR1A | _BV(COM1A1)) & ~_BV(COM1A0);
  TCCR1A &= ~(_BV(COM1B1) | _BV(COM1B0));
  TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);
  
  // Set initial pulse width to the first sample.
  OCR1A = 0;
  // Set up Timer 1 to send a sample every interrupt.
  cli();
  TCCR2B = (TCCR2B & ~_BV(WGM13)) | _BV(WGM12 );
  TCCR2A = TCCR2A & ~(_BV(WGM11) | _BV(WGM10) );
  TCCR2B = (TCCR2B & ~(_BV(CS22) | _BV(CS21))) | _BV(CS20);
  OCR1B = F_CPU / SRATE;    // 16e6 / 8000 = 2000
  TIMSK1 |= _BV(OCIE1B);
  sei();
  pinMode(9, OUTPUT);
  
}

/* CTC PWM on pin 1
 * http://neilmb.github.io/cheapnewham/2015/07/21/Arduino-IQ-Audio-Frequency-DDS.html 
 */
void pwmSetup() {
  
  ASSR &= ~(_BV(EXCLK) | _BV(AS2));
  
  TCCR2A |= _BV(WGM21) | _BV(WGM20);
  TCCR2B &= ~_BV(WGM22);

  // Do non-inverting PWM on pin OC2A (p.155), p11
  TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
  TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));
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


inline void setLimits(byte a1, byte a2, byte b1, byte b2, byte c1, byte c2) {
      aMax = a2; aMin = a1; 
      bMax = b2; bMin = b1;
      cMax = c2; cMin = c1;
}

/* pentatonic
 *  
 *  A(t) = 1
    C(t) = 32/27
    D(t) = 4/3
    E(t) = 3/2
    G(t) = 16/9
 */
 /* 
 *  I, Mark Washeim do not assert any Copyright for the result formulas. They belong to the public domain
 *  when no other copyright has been asserted.
 *  
 *  Various bytebeat formulas. some from my old midi/arduboy sketches, nyblybyte has a lot of references
 *  https://github.com/erlehmann/algorithmic-symphonies and
 *  https://raw.githubusercontent.com/schollz/nyblcore/main/bytebeat/bytebeat.ino
 */

// return t*(((t>>11)^((t>>11)-1)^7)%11)
// t*((0xDEADBEEF>>((t>>11)&37)&7)*(1/4)*(0xFEFEED>>((t>>13)&42)&5))
// birthday fades in and out
//       result = ~t>>11 ^ a | ( (t>>a)&(t<<b) * 1968|(t>>c)%1976 ) / ( t%(a+b+c) | t%2012)  ;
// from here nybly see the header file for information
// nice 8, 17, ( bb28 | t << c), bb32 << c (or |), 34 great, 37 ( also  << c, | c), b39 ( | t>>c)
// normal bb0,bb4 bb11 ( bb19 bb21 | t << c) (bb22 + c) bb23 + c, bb33 << c, bb35 << c, 
// 8192 slow bb5 bb7 bb9 (bb13 | c) bb14 bb16 (bb29 & t>>c), bb30 << c, bb36 | t <<  c;

 
void rythmical(int pb1) {
  
  switch (pb1) {
     case 1:  // poetaster the next three together
      //setLimits(1,8,1,8,1,8); // aMin, aMax, etc
      setLimits(1,32,1,16,1,9); // aMin, aMax, etc
      //result = ( ~t >>7? ~t>>3:~t>>5) * ( (127 & t * ( b & t >> 9) ) < (254 & t * ( 2 + ( c & t >> a ) ) ) );
      result = t >> c ^ t & 1 | t + (t ^ t >> 13) - t * ( (t >> 5 ? b : a) & t >> ( 8 - ( a >> 1 )  ) );  // ( t%8 ? ~t >> 9 & b: ~t >>11 & c )
      break;
    case 2:  
      setLimits(1,16,1,16,1,16); 
      result =  t >> 4 ^ t & c | t + (t ^ t >> 8) - t * ((t >> 3 ? b : a) & t >> ( 5 - ( b >> 1 )  ) ); 
      // ^ t << 1 & (a & 12 ? t >> 4 : t >> 10)
      break;
    case 3:
      setLimits(1,16,1,16,1,16);
      result = t >> (2*a) ^ t & c | t + (t ^ t >> 21) - t * ((t % 8 ? b : a) & t >> ( 8 - ( b >> 5 )  ) ); 
      break; 

    case 4: //  poetaster 
      setLimits(0,16,0,16,0,16);
      //if (t > 65536) t = -65536;
      result= ((t%32 ? ~t>>2 : t>>3) & t + (t >> a) + ( ~ (t ^ b/2| t >> c) )  );
      
      //result=  ((t%4 ? c : b) & ((t<<1)^-(t>>a&1) &  ( 127 << (t+ b) | (t >> c)));
      break;
            
    case 5:  // always sirens noisy based on https://www.pouet.net/topic.php?post=388938
      setLimits(1,16,1,16,1,16); // aMin, aMax, etc
      result =  (t>>3 & a) + (t*( 0xC4 + a >> ( t>> 3 % b) & (13 + c) )| t >> c ) + (int)cos( t * 0xC4 + b);
//      techno result = t ^ ( t >> c) - t | t >> a * t<<1 & (t&3 ? t >> b: t >> 13 ) ^ ( (t >> 3 ? 8 : 3 ) & t >> 128);

    case 6:   // poetaster the devil. pulses and mana, mana.three controls can cross conflict or harmonize nice. turn c up first
      setLimits(1,30,1,15,1,30);
      result = t *  t << 1 & (t & 7 ? t >> 3 : t >> c) ^ ((t >> 7 ? 2 : b) & t >> (c + a)) | t + ((t ^ t >> 13)) | a * t >> b ^ t & c ;
      break;
    case 7:  // poetaster, noisy bassline , arps with tight snare  ... evolves. needs work? 
      setLimits(1,16,1,41,1,30);
      result = t * ((t >> 7 ? a : c ) & t >> a ) ^ t << 1 & (t & 7 ? t >> 5 : t >> c) - b * t >> 3 ^ t & (42 - b) ;
      break;
    case 8: // started with vizmut paper pp. 6 https://arxiv.org/pdf/1112.1368.pdf // modified to keep from crashing :) divisions replaced with multiplication :)
      setLimits(0,32,0,32,0,32);
      result = t >> c |  t & (  ( t >> 5 ) / ( t >> b*4 - ( t >> a/3 ) & - t >> b/4 - ( t >> a/3 ) ) );
      // t >>4 | t &(( t >> 5 )/( t >> 7 − ( t >> 15 ) & − t >> 7 − ( t >> 15 ) ) )
      break;

    case 9:  // variation t+(t&1)+(t>>5)*(t>>1)/1|t>>4|t>>8  nice bass drones with whirring open hats? or damped cymbals https://dollchan.net/btb/res/3.html#258
      setLimits(4,37,0,16,5,12);
      result = ( t + ( t & b) + ( t >> a ) * ( t >> b ) / 1 | t >> b | t >> c );
      break;
    case 10: // scratch percussion on th extreme (try * c)
      setLimits(1,68,1,68,0,32);
      bb37_set(a,b); 
      result =  bb37() & t >> c;
      break; 
    case 11: // 5824 hz is nice. quaks like a duck
      setLimits(1,16,1,16,0,32);
      bb22_set(a,b); 
      result =  bb22() -c ;
      break;
    case 12:     // Nightmachines https://forum.aemodular.com/post/235
      setLimits(1,16,1,16,0,16);
      result  = (t * 4 | t | t >> 3 & t + t / 4 & t * a | t * 8 >> b | t / c & t + 140) & t >> 4;
      break;
    case 13: // http://arhivach.ng/thread/28592/#71678984 // crashes??? this is not the original, but, still :)
      setLimits(2,64,2,8,0,16);      
      result = t >> b + t % a | t >> c + t - ( t ^ (t / 31108 & 1 ? (46 -c) : (43 - c )) ) | t / b | t / c % a; // % a crashes
      break;   
    case 14:  // poetaster this is great with a modulator like the kastle.
      setLimits(0,8,0,8,0,8);
      result = ( t%16 ? t >> a & t >> 7 : t>> 11 & t >> b ) * ( t%8 ? ~t >> 9 & b: ~t >>11 & c ) & 64 ; 
      break;
    case 15:  // https://forum.arduino.cc/t/one-line-algorithmic-music/73409
      // (t*(4|t>>13&3)>>(~t>>11&1)&128|t*(t>>11&t>>13)*(~t>>9&3)&127)^(t&4096?(t*(t^t%255)|t>>4)>>1:t>>3|(t&8192?t<<2:t))
      setLimits(0,16,0,16,0,16);
      result =  ( t & (a*16) ? ( t * ( t ^ t % (b*8) ) | t >> a ) >> 127 : t >> b |( t & (c*32) ? t << c : t ) )   ;
      break;
    case 16: // started with vizmut paper pp. 6 https://arxiv.org/pdf/1112.1368.pdf // modified to keep from crashing :) divisions replaced with multiplication :)
      setLimits(0,16,0,16,0,16);
      result = t >> c | t | (( t >> 5 )/( t%2 ? t >> b/2: t>> b)) | ( t%3 ? t >> a/3: t>> a ); 
      break;
    case 17: // nice
      setLimits(1,68,1,68,1,32);
      bb34_set(a,b); 
      result =  bb34() * c; // or - c
      break;
  }
}

void melodious(int pb2){
  
  switch (pb2) {
    case 1:  // https://forum.arduino.cc/t/one-line-algorithmic-music/73409
      // (t*(4|t>>13&3)>>(~t>>11&1)&128|t*(t>>11&t>>13)*(~t>>9&3)&127)^(t&4096?(t*(t^t%255)|t>>4)>>1:t>>3|(t&8192?t<<2:t))
      setLimits(4,19,0,7,0,14);
      result = (t * (4 | t >> 13 & b ) >> ( ~t >> 11 & 1 ) & 128 | t * ( t >> a & t >> 13 ) * ( ~t >> c & 3 ) & 127 ) ^ ( t & 4096 ? ( t * ( t ^ t % 255 ) | t >> 4 ) >> 1 : t >> 3 |( t & 8192 ? t << 2 : t ) );
      break;
    case 2:  // FIX this is actually rythm https://forum.arduino.cc/t/one-line-algorithmic-music/73409
      // (t*(4|t>>13&3)>>(~t>>11&1)&128|t*(t>>11&t>>13)*(~t>>9&3)&127)^(t&4096?(t*(t^t%255)|t>>4)>>1:t>>3|(t&8192?t<<2:t))
      setLimits(0,16,0,16,0,16);
      result =  ( t & 64 ? ( t * ( t ^ t % 128 ) | t >> a ) >> 127 : t >> b |( t & 32 ? t << c : t ) )   ;
      break;
    case 29:  // xpansive 2011-09-29 https://www.pouet.net/topic.php?which=8357&page=3#c388375
      // t * (t >> 8 | t >> 9) & 46 & t >> 8 ^ (t & t >> 13 | t >> 6);
      setLimits(40,56,1,9,0,16);
      result = t * (t >> 8 | t >> 9) & a & t >> 8 ^ (t & t >> c | t >> b);
      break;
    case 3: // FIX rythm straight rythmic, great range! // tejeez 2011-10-05 #countercomplex  
      setLimits(1,16,1,16,0,16);
      result = ( ~t >> 2) * ((127 & t * (b & t >> 10)) < (245 & t * (2 + (c & t >> a))));
      break;
   case 4: // variation on 45
      setLimits(0,32,0,32,0,32); // aMin, aMax, etc
      result = ( ~t >> 2? ~t>>3:~t>>2) * ( (127 & t * ( b & t >> 10) ) < (245 & t * ( 2 + ( c & t >> a ) ) ) );
      break;
   case 5:  // variation on 45
      setLimits(0,32,0,32,0,32); // aMin, aMax, etc
      result = ( ~t >>7? ~t>>3:~t>>4) * ( (245 & t * ( b & t >> 10) ) < (127 & t * ( 2 + ( c & t >> a ) ) ) );
      break;
    case 6:  //  FIX rythm? Street Surfer by skurk, raer (2011-09-30) https://www.pouet.net/topic.php?which=8357&page=4#c388479
      setLimits(1,16,1,16,0,16);
      result  = t & (4096) ? t / 2 * (t ^ t % (a << 1)) | t >> 5 : t / (a << 1) | (t & (b << 7) ? 4 * t : t);
      break;
    case 7:  // poetaster long distorted carnival number
      setLimits(1,12,1,16,1,16);
      result = (t * a & t >> b | t * c & t >> 7 | t * 3 & t / 1024) - 1; 
      break; 
// maybe also in rythmical?
    case 8:  // poetaster twisted calliope distortion can rock :)
      setLimits(1,30,1,15,1,15);  
      result = (  t * b & c / (a << 2) | t * b & t >> c | t * ( c+b ) & t >> a) - 1;;
      break;
    case 9: // funny melody! and arps.  http://www.pouet.net/topic.php?which=8357&page=9#c388930",
      setLimits(1,16,1,16,0,32);
      result =   ( t * ( ( a + ( 1 ^ t >> 10 & b ) ) * ( b + ( a & t >> 14 ) ) ) ) | ( t >> c  );
      break;
    case 10: // FIX rythm, broken poetaster fast to slow arps + staccato noise and robot voice = breaks with 9 o'clock, 5 to 12 o'clock, 3 o'clock
      setLimits(0,64,0,16,0,16);
      result = (( t >> c | t << b ) & (a + 1))  * (( t >> (b + 1) ) | t >> ( t << 21)) ;
      break;    
    case 11:  // poetaster pulse drone // seems to kill the nano after all?
      setLimits(1,32,1,24,1,16);
      result = ((t*a) & ( t>>5| t<<2 )  ) | ( (t*b) & ( t>>4 | t<<3)) | ((t*c) & ( t>>3 | t<<4 ) );
      break; 
    case 12:  // a melodic drone
      setLimits(8,16,7,14,6,12);
      result = t - b & ( (t>>a | t<<4 ) ) ^ t - c & ( ( t>>b | t<<3 ) ) ^ t - a & ( ( t>>c | t<<2 ) ) ;
      break;
    case 13:  // poeaster variation with melodic with shells  variation on https://dollchan.net/btb/res/3.html#78
       // ( ( t >> 10 | t * 5 ) & ( t >> 8 | t * 4 ) & ( t >> 4 | t * 6 ) );
      setLimits(0,16,0,8,0,16);
      result = ( ( t >> a | t * 5 ) & ( t >> ( a + 2 ) | t * b ) & ( t >> b | t * c ) );
      break;
    case 14: // try * c?
      setLimits(1,68,1,68,0,32);
      bb32_set(a,b); 
      result =  bb32() | t >> c;
      break; 
    case 15:
      setLimits(1,68,1,68,0,32);
      bb8_set(a,b); 
      result =  bb8() | t >> c;
      break; 
    case 16: // pad melodies 2 & 3 part with noise crackle on c
      setLimits(1,68,1,68,0,32);
      bb35_set(a,b); 
      result =  bb35() - c ;
      break;
    case 17:      // from http://xifeng.weebly.com/bytebeats.html we have someting similar already
      setLimits(1,16,1,16,0,16);
      result  = ((((((((t >> a) | t) | (t >> a)) * c) & ((5 * t) | (t >> c))) | (t ^ (t % b))) & 0xFF));
      break;
    case 18: // same as 49 origin. more 4/4 tech qwowloon can be metal guitar
      setLimits(1,32,1,32,1,32);
      result =   (t*((3 + ( 1 ^ t >> a & 5 ) ) * ( 5 + ( 3 & t >> b ) ) ) ) >> ( t >> c & 3 );
      //result = t >> c ^ t & 1 | t + (t ^ t >> 13) - t * ((t >> 5 ? b : a) & t >> ( 8 - ( a >> 1 )  ) );
      break; 
    case 19:   // poetaster drone, organ, perc
      setLimits(1,8,1,16,1,8);
      result= ( ( t * a & t >> 4 ) | ( t * b & t >> 7 ) | ( t * c &  t) ) - ( t >> b ? 13 : 8)   ;
      break;  
    case 20: //poetaster requires 4* normal speed
      setLimits(32,39,32,47, 32,58);
      result = t * ( ( ( t>>11*a/27 ) & ( ( t >> 11*b/27 ) - 1 ) ^ 7) * 11*c/27);
      //result = t * ( ( ( t%2? t>>11*a/27: t>>11*a/9 ) ^ ( ( t >> 11*b/9 ) - 1 ) ^ 7) ^ 11*c/9);
    case 21: // http://entropedia.co.uk/generative_music/#v3b64K0otKS3KU9AoUdMosbMz09TWKLGx0QCxDQ014yAMY01tY03NGgjHRNXMBKQKxNTU1FTT0AUyTTU1rRX0tRRCihKTsxWcixLTSioVkioVAhJLcxQ8EitTixW09AE=
      setLimits(1,8,11,16,1,24);
      result = (t&(t>>6)+(t<<((t>>11)^((t>>b)+a))|((t>>c%64)+(t>>c)))&(-t>>5));
      //result = (t&(t>>6)+(t<<((t>>11)^((t>>13)+3))|((t>>14%64)+(t>>14)))&(-t>>5));
  }
  
}

void noisy(int pb3){
  
    switch (pb3) {
    case 1:  // poetaster helicopters has some arps with b in the middle, various! can also humm with bells.
      setLimits(8,24,1,16,1,16);
      result = ( (t >> a) - ( t >> a & t )  + ( t >> t &  b ) ) + ( t * ((t >> c) & c ) );
      break;
    case 2:   // poetaster windy whirls, noisy swirls, ocean swells, drop off. revisit since it funks as
      setLimits(1,12,1,16,1,16);
      result =  t >> ( ( a + 1 )  & b & t >> 8 ) ^ ( t & t >> a | t >> c ); /// maybe t >> 16?
      break;
    case 3:  // poetaster breaky, jungle stuff a at one oclock, c middle, etc
      setLimits(1,12,1,16,1,16); 
      if (t > 65536) t = -65536; 
      result = a + ( ( t >> a + 1 ) ) * (t >> c | b | t >> ( t >> 16) )  ;
      break;
    case 4:  // poetaster techno stuff but slow it down and it's rockin
      setLimits(1,8,1,10,1,10);
      result =  (t & t >> b | t << b >> c) ^ ( t  &  t >> a | t << a >> b) & ( t & t >> c | t << c >> a);
      break;    
     case 5: // poetaster counter point bleeps with a hihat or reverb stuff NAH
      setLimits(1,30,1,30,1,30);
      result =  ( t  &  t >> c | t + c << 4) | (t & t >> a | t + a << 3) ^ ( t & t >> b | t + b << 2 );
      break; 
    case 6: // poetaster - sound fx wroom zoom ..... goes long not like th original nice variety http://www.pouet.net/topic.php?which=8357&page=17#c389829",
      // (t*(t>>12)*64+(t>>1)*(t>>10)*(t>>11)*48)>>(((t>>16)|(t>>17))&1
      setLimits(1,30,1,15,1,15);
      result = ( t * (t>>c) * 64 + ( t >> 1 ) * ( t >> b ) * (t >>7) * 48 ) >> ( ( (t>>16)|(t>>a) )+1);
      break; 
    case 7:   // poetaster c high, techno wheep, b pitch in the middle, a patterned blips fast to slow
      setLimits(1,24,1,12,1,16);  
      result =  t * ( t >> b & ( t >> b ? 13 : 8)  ) - t >> c ^ t & 21 | t + (t ^ t >> a)  ^ t<<1 & (t & a ? t >> 5 : t >> 3);
      break;     
    case 8:  // poetaster descending / ascending bleep arps, lasers, a high bass run
      result = t - ((t & ((t >> a))) + ( a | t >> c )) & (t >> ( c + 1)) | (t >> b) & (t * (t >> a));
      break;
    case 9:  // poetaster also a drone, but melodic basic with perc blurbs hh hits goes to techno with 3 part development
      setLimits(4,16,3,16,1,16);
      result =  ( t >> a | t - b ) & ( t -a | t >> b ) * c;   
      break;
    case 10:   // started with vizmut paper pp. 5 https://arxiv.org/pdf/1112.1368.pdf = hacked to bits :)
      setLimits(2,100,2,50,2,10);
      result = t - b & ( ( t >> a | t >> b ) ) ^ t - c & ( ( t>> b | t << c*2 ) ) ^ t - a & ( ( t>> c | t >> 3 ) ) ;
      break;
    case 11:    // started with vizmut paper pp. 6 https://arxiv.org/pdf/1112.1368.pdf
      setLimits(1,16,1,16,1,16);
      result = t - b & ( (t>>a | t<<4 ) ) ^ t - c & ( ( t>>b | t<<3 ) ) ^ t - a & ( ( t>>c | t<<2 ) ) ;
      //result = (int)(t/1e7 * t * t + c ) % 127 | t >> c ^ t >> b | t % 127 + ( t >> a ) | t ;
      break;
    // the bb funcs are from nyblybyte.h
    case 12:  // b39 crashes? or just really slow? bb39 is nice but no go
      setLimits(1,68,1,68,0,32);
      bb19_set( a,b); 
      result =  bb19() | t >> c ;
      break;
    case 13: // it evolves noisy. lots of breaks to silence
      setLimits(1,68,1,68,1,16);
      bb28_set(a,b); 
      result =  bb28() |  ~t << c;
      break;
    case 14: // yeah, it's nice and evolves a lot :)
      setLimits(1,68,1,68,0,32);
      bb17_set(a,b); 
      result =  bb17() | t >> c;
      break;
      
    case 15: // no needs 8000 hz and takes a while // actually melodic
      setLimits(1,16,1,16,0,32);
      bb23_set(a,b); 
      result =  bb23() - c;
      break;  
    case 16:// ?? borked
      setLimits(1,16,1,16,0,16);
      bb33_set(a,b); 
      result =  bb33() *  c;
      break; 
    case 17: // bit boring :) //https://www.blogger.com/profile/02935728280207314233 
      setLimits(0,8,0,8,0,8);
      result = (t<16384)?(t*a&t*2>>b):(t<32768) ?(t*a&t*2>>c):(t<49152) ?(t*(b)&t*2>>a):(t<65536) ?(t*(b)&t*2>>c):0 ;
      if(t>=65536)t=0;
    case 18:  // poetaster a synth lead with breaks, and stuff :) c 12 oclock and it starts being a wave form 
      setLimits(1,16,21,63,0,16);
      result= (t >> 3 ? b : a) * t >> c ^ t % c | t + (t ^ t >> a) ; 
    case 19:  // poetaster mad techno helicopter bird flock this is an evil hack.
      setLimits(1,16,1,16,0,16);
      result =    ( t / ( a * b ) ) & ( ( t >> a ) & t >> b ) * ( t << c) + a ^ ( t >> (t / b ) ) ;//^ ( t >> ( 16 * c ) ) ;
      break;
    case 20:  // poetaster clicky burpy can do techno ! it's great when you find it ;)
      setLimits(0,15,0,11,0,9);
      result = ( ( (t >> 9 ? a : b) & t * (t >> b) % (t >> c) - (t >> b) ) * (t >> a) ) ;
      break;

  }
}

ISR(TIMER1_COMPA_vect) {

  switch (bank) {
    case 1:
      rythmical(pb1);
      break;
    case 2:
      melodious(pb2);
      break;
    case 3:
      noisy(pb3);
      break;
  }

  //PWM16B(result);
  OCR2A =  result;
  if ( enc_offset != 0) t += enc_offset;

  
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
