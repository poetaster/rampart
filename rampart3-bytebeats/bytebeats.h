#pragma once

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


inline void PWM16A(unsigned int PWMValue)
{
  OCR1A = constrain(PWMValue, 0, TOP);
}


inline void PWM16B(unsigned int PWMValue)
{
  OCR1B = constrain(PWMValue, 0, TOP);
}

/* 
 *  These are the original methods from Glitchstorm, plus a few of mine and from
 *  https://github.com/erlehmann/algorithmic-symphonies
 */

ISR(TIMER1_COMPA_vect) {

  switch (programNumber) {
    case 1:
      aTop = 10;
      aBottom = 0;
      bTop = 14;
      bBottom = 0;
      cTop = 14;
      cBottom = 0;
      value = ((t & ((t >> a))) + (t | ((t >> b)))) & (t >> (c + 1)) | (t >> a) & (t * (t >> b));
      break;
    case 2:
      aTop = 12;
      aBottom = 0;
      bTop = 20;
      bBottom = 4;
      cTop = 12;
      cBottom = 5;
      if (t > 65536) t = -65536;
      value = (t >> c | a | t >> (t >> 16)) * b + ((t >> (b + 1)) & (a + 1));
      break;
    case 3:
      aTop = 30;
      aBottom = 6;
      bTop = 16;
      bBottom = 0;
      cTop = 10;
      cBottom = 0;
      //value = t>>6^t&37|t+(t^t>>11)-t*((t%a?2:6)&t>>11)^t<<1&(t&b?t>>4:t>>10);
      value = t >> c ^ t & 37 | t + (t ^ t >> a) - t * ((t >> a ? 2 : 6)&t >> b)^t << 1 & (t & b ? t >> 4 : t >> 10);
      break;
    case 4:
      aTop = 12;
      aBottom = 0;
      bTop = 16;
      bBottom = 0;
      cTop = 10;
      cBottom = 0;
      // value = t>>6^t&37|t+(t^t>>11)-t*((t%a?2:6)&t>>11)^t<<1&(t&b?t>>4:t>>10);
      value = b * t >> a ^ t & (37 - c) | t + ((t ^ t >> 11)) - t * ((t >> 6 ? 2 : a)&t >> (c + b))^t << 1 & (t & 6 ? t >> 4 : t >> c);
      break;
    case 5:
      aTop = 24;
      aBottom = 0;
      bTop = 22;
      bBottom = 0;
      cTop = 16;
      cBottom = 0;
      // t>>6^t&37|t+(t^t>>11)-t*((t%a?2:6)&t>>11)^t<<1&(t&b?t>>4:t>>10);
      //value = t+(t&t^t>>6)-t*((t>>9)&(t%16?2:6)&t>>9)
      // value = t+(t&t^t>>(b*2-c))-t*((t>>a)&(t%c?2:(a-c))&t>>b);
      value = c * t >> 2 ^ t & (30 - b) | t + ((t ^ t >> b)) - t * ((t >> 6 ? a : c)&t >> (a))^t << 1 & (t & b ? t >> 4 : t >> c);
      break;
    case 6:
      aTop = 10;
      aBottom = 3;
      bTop = 28;
      bBottom = 0;
      cTop = 10;
      cBottom = 3;
      //value = ((t>>a&t)-(t>>a)+(t>>a&t))+(t*((t>>b)&b));
      value = ((t >> a & t) - (t >> a) + (t >> a & t)) + (t * ((t >> c)&b));
      break;
    case 7:
      aTop = 10;
      aBottom = 0;
      bTop = 22;
      bBottom = 10;
      cTop = 8;
      cBottom = 0;
      //SE CUELGA A 16KH
      // value = ((t % 42 + b) * (a >> t) | (128 & b) - (t >> a)) % (t >> b) ^ (t & (t >> c));
      //value =  t>>b&t?t>>a:-t>>c ;
      value =  t >> b & t ? t >> a : -t >> c ;
      break;
    case 8:
      //16kh only work by 65536 loops
      aTop = 12;
      aBottom = 0;
      bTop = 20;
      bBottom = 0;
      cTop = 20;
      cBottom = 0;
      if (t > 65536) t = -65536;
      value = (t >> a | c | t >> (t >> 16)) * b + ((t >> (b + 1)));
      break;
    case 9:
      // value = ((t*(t>>a|t>>(a+1))&b&t>>8))^(t&t>>13|t>>6);
      aTop = 32;
      aBottom = 1;
      bTop = 32;
      bBottom = 1;
      cTop = 32;
      cBottom = 1;
      value = ( ( t * (t >> a | t >> ( b & t) ) & t >> 8) + 1 ) ^ (t & t >> c | t >> 6);
      //( ( t / 9 >> a ) ^ ( t / 6 >> b ) & ( t / 3 >> c) * t ) >> 8 ;
      
      break;
    case 10:
      aTop = 12;
      aBottom = 1;
      bTop = 12;
      bBottom = 1;
      cTop = 12;
      cBottom = 1;
      //if (t > 65536) t = -65536;
      value = (t*(a+(t/131072%2))*(t>33e3) & t>>4 | t*(c+(t/32768%2)) & t>>7 | t*(b+(t/65536%2)) & (t>32768)&t>>11)-(t>97e3);
      break;
    case 11:
      aTop = 16;
      aBottom = 1;
      bTop = 16;
      bBottom = 1;
      cTop = 16;
      cBottom = 1;
      value = (t * a & t >> b | t * c & t >> 7 | t * 3 & t / 1024) - 1; 
      // % is a too heavy operatin for atmel 328. Should not be used in any equation. Change it
      // value = ((t >> a % (128-b<<t))) * b * t >>( c*t<<4) * t >> 18 ;
      //DEFFO
      //((t >> a / (128 - b << (t >> (9 - c))))) * b * t >> ( c * t << 4) * t >> 18 ;
      // value = ((t >> a % (128-b<<(t>>(9-c))))) * b * t >>( c*t<<4) * t >> 18+(t >> c ? 2 : a)&t * (t >> b) ;
      // value = ((t >> 6 ? 2 : a)&t * (t >> c) | ( b) - (t >> a)) % (t >> b) + (4 | (t >> c));
      // value = ((t >> b ? c : a)&t * (a) | ( 8) - (t >> 1)) % (t >> b) + (4 | (t >> c));

      break;
    case 12:
      // moola long
      aTop = 24;
      aBottom = 0;
      bTop = 24 ;
      bBottom = 1;
      cTop = 14;
      cBottom = 1;
      value = (t * 12 & t >> a | t * b & t >> c | t * b & c / (b << 2)) - 2;
      //value = (t * a & t >> b | t * c & t >> 7 | t * 3 & t / 1024) - 1;
      break;
    case 13:
      //moola viznu
      aTop = 18;
      aBottom = 10;
      bTop = 14;
      bBottom = 1;
      cTop = 10;
      cBottom = 1;
      //value = (t * 5 & t >> 7) | (t * 3 & t >> 10);
      value = ((t * (t >> a) & (b * t >> 7) & (8 * t >> c)));
      break;

    case 14:
      aTop = 8;
      aBottom = 0;
      bTop = 16;
      bBottom = 0;
      cTop = 1;
      cBottom = 6;
      //Both cool. What to choose?
      //value=t >> c ^ t & 1 | t + (t ^ t >>21) - t* (( t >> 4 ? b : a) &t >> 12 ) ^ t << 1 & ( a & 12 ? t >> 4 : t >> 10 );
      value = t >> c ^ t & 1 | t + (t ^ t >> 21) - t * ((t >> 4 ? b : a)&t >> (12 - (a >> 1)))^t << 1 & (a & 12 ? t >> 4 : t >> 10);
      break;
    case 15:
      aTop = 8;
      aBottom = 0;
      bTop = 9;
      bBottom = 0;
      cTop = 5;
      cBottom = 0;
      value = ((t &  (4 << a)) ? ((-t * (t ^ t ) | (t >> b)) >> c) : (t >> 4) | ((t & (c << b)) ? t << 1 : t));
      //value = (t*((t>>a|t<<c)&29&t>>b));
      //((t&4096)?((t*(t^t%255)|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t))
      // value = ((t & (4 << c)) ? ((t * (t ^ t & a) | (t >> b)) >> 1) : (t >> 4) | ((t & (c << b)) ? t << 1 : t));
      //value = ((t &  (4 << a)) ? ((-t * (t ^ t ) | (t >> b)) >> c) : (t >> 4) | ((t & (c << b)) ? t << 1 : t));
      break;
    case 16:
      aTop = 8;
      aBottom = 0;
      bTop = 9;
      bBottom = 0;
      cTop = 6;
      cBottom = 0;
      value = ((t &  (4 << a)) ? ((-t * (t ^ t ) | (t >> b)) >> 3) : (t >> c) | ((t & (3 << b)) ? t << 1 : t));
      break;
    case 17:
    // pulse drone
      aTop = 32;
      aBottom = 1;
      bTop = 24;
      bBottom = 0;
      cTop = 16;
      cBottom = 0;
      value = ((t*a) & ( t>>5| t<<2 )  ) | ( (t*b) & ( t>>4 | t<<3)) | ((t*c)/2 & ( t>>3 | t<<4 ) );
      // kills a nano 
      // (t*8 & (t/24>>5|t<<2)) | (t*6 & (t/18>>4|t<<3)) | (t*4 & (t/12>>3|t<<4))
      break;
    case 18:
    // drone, organ, perc
      aTop = 8;
      aBottom = 1;
      bTop = 16;
      bBottom = 1;
      cTop = 8;
      cBottom = 1;
      value= ( ( t * a & t >> 4 ) | ( t * b & t >> 7 ) | ( t * c &  t) ) - 1;
      // nah :)
      //(1/(128000-t))|(t>96e3)?t%4000*("'&(&*$,*"[t%96000/4000]-a):(t%2000*("$$$&%%%''''%%%'&"[t%32000/2000]-b-(2*((t>28e3)&(t<32e3)))))/(1+(t%8000<4e3));
      break;
    case 19:
    // also a drone, basic with perc blurbs
      aTop = 16;
      aBottom = 4;
      bTop = 16;
      bBottom = 3;
      cTop = 16;
      cBottom = 1;
      value =  ( t >> a | t - b ) & ( t -a | t >> b ) * c;   
      break;
    case 20:
    // a melodic drone
      aTop = 16;
      aBottom = 8;
      bTop = 14;
      bBottom = 7;
      cTop = 12;
      cBottom = 6;
      //value = ((t>>32)*7|(t>>a)*8|(t>>b)*7)&(t>>7); 
      // 
      value = t - b & ( (t>>a | t<<4 ) ) ^ t - c & ( ( t>>b | t<<3 ) ) ^ t - a & ( ( t>>c | t<<2 ) ) ;
      
      //( (  (t/6) >> a &t) + ( t << 4) ) & ( ( ( ( t/3 ) >> b & t) + ( t << c) ) );
      break;

       
      //((i * (i >> 8 + i >> 9) * 100) + sin(i));

  }

  //PWM16B(value);
  OCR2A =  value;
  t += shift_A_Pot;

  // timing  clockout easter-egg mode
  cyclebyte++;
  if (cyclebyte == 1024 ) {
    cyclebyte = 0;
    if (isClockOutMode ) {
      //ledCounter();
    }
  }


}






void initSound()
{
  pinMode(speakerPin, OUTPUT);

  ASSR &= ~(_BV(EXCLK) | _BV(AS2));
  
  TCCR2A |= _BV(WGM21) | _BV(WGM20);
  TCCR2B &= ~_BV(WGM22);

  // Do non-inverting PWM on pin OC2A (p.155)
  // On the Arduino this is pin 11.
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
  TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
  TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));

  // No prescaler (p.134)
  TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

  // Set the compare register (OCR1A).
  // OCR1A is a 16-bit register, so we have to do this with
  // interrupts disabled to be safe.
  OCR1A = F_CPU / SAMPLE_RATE;    // 16e6 / 8000 = 2000
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
