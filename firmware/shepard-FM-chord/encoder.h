
#include <EncoderButton.h>
// encoder
// the a and b + the button pin
EncoderButton eb1(5, 6, 4);
// the library has nice button handling and uses bounce2
EncoderButton left(3); // pin 3 rampart
EncoderButton right(2); // pin 2 rampart


int encoder_pos_last = 0;
long encoder_delta = 0;
int enc_offset = 1; // changes direction
int enc_delta; // which direction


/**
   handle encoder button long press event
*/
void onEb1LongPress(EncoderButton& eb) {
  
  if (debug) {
    Serial.print("button1 longPressCount: ");
    Serial.println(eb.longPressCount());
    Serial.println(mode);
    
  }
}
/**
   handle encoder turn with  button pressed
   offsets OCR2A
*/
void onEb1PressTurn(EncoderButton& eb) {
  enc_delta = eb.increment();

  int dir = enc_offset + eb.increment();
  dir = constrain(dir, -7, 7 );

  enc_offset = dir;
  if (debug) {
    Serial.print("eb1 press inc by: ");
    Serial.println(eb.increment());
    Serial.print("enc_offset is: ");
    Serial.println(enc_offset);
  }
}

/**
   handle encoder turn with  button pressed
*/
void onEb1Clicked(EncoderButton& eb) {


  if (debug) {
    Serial.print("bank: ");
    Serial.println(eb.clickCount());
  }
  // displayUpdate();
}

/**
    handle left button short release
*/
void onLeftReleased(EncoderButton& left) {
  //buttonState = 0;
  mode = mode-1;
  if (mode < 0) mode = 0;
  if (debug) {
    Serial.print("PROGRAM: ");
    Serial.println(mode);
    Serial.println("left");
  }
}

/**
    handle right button short release
*/
void onRightReleased(EncoderButton& right) {
  //buttonState = 1;
    mode = mode+1;
  if (mode > 2) mode = 2;
  if (debug) {
    Serial.print("PROGRAM: ");
    Serial.println("right");
    Serial.println(mode);
  }
}

/**
   A function to handle the 'encoder' event without button
*/
void onEb1Encoder(EncoderButton& eb) {

  //displayUpdate();
  encoder_delta = eb.increment();
 /* int cstep = eb.increment() + lFreq;
  if (cstep != lFreq ) {
    lFreq = cstep;
    kLfo.setFreq(constrain(lFreq,0,100));
  }*/
  
  if (debug) {
    Serial.print("eb1 incremented by: ");
    Serial.println(eb.increment());
    Serial.print("eb1 position is: ");
    //Serial.println(lFreq);
  }
}
