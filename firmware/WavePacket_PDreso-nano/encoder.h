#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <EncoderButton.h>
// encoder
// the a and b + the button pin
EncoderButton eb1(6, 5, 4);
// the library has nice button handling and uses bounce2
EncoderButton left(3); // pin 3 rampart
EncoderButton right(2); // pin 2 rampart


int encoder_pos_last = 0;
long encoder_delta = 0;
int enc_offset = 1; // changes direction
int enc_delta; // which direction
int prog = 1;
int bank = 1;
int pb1 = 1;
int pb1total = 16;
int pb2 = 1;
int pb2total = 21;
int pb3 = 1;
int pb3total = 21;
int numProg = 52;

/**
   handle encoder button long press event
*/
void onEb1LongPress(EncoderButton& eb) {
  buttonState = !buttonState;
  if (debug) {
    Serial.print("button1 longPressCount: ");
    Serial.println(eb.longPressCount());
    Serial.println(buttonState);
    
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

  // set which bank to select formulas from
  bank = eb.clickCount();

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
  buttonState = 0;
  
  if (bank == 1)
  {
    if (pb1 > 1) {
      pb1--;
    } else if (pb1 == 1) {
      pb1 = pb1total;
    }
    prog = pb1;
  } 
  else if (bank == 2) {
    if (pb2 > 1) {
      pb2--;
    } else if (pb2 == 1) {
      pb2 = pb2total;
    }
    prog = pb2;
  } 
  else if (bank == 3) {
    if (pb3 > 1) {
      pb3--;
    } else if (pb3 == 1) {
      pb3 = pb3total;
    }
    prog = pb3;
  }

  if (debug) {
    Serial.print("PROGRAM: ");
    Serial.println(buttonState);
    Serial.println("left");
  }
}

/**
    handle right button short release
*/
void onRightReleased(EncoderButton& right) {
  buttonState = 1;
  if (bank == 1)
  {
    if (pb1 < pb1total) {
      pb1++;
    } else if (pb1 == pb1total) {
      pb1 = 1;
    }
    prog = pb1;
  } 
  else if (bank == 2) {
    if (pb2 < pb2total) {
      pb2++;
    } else if (pb2 == pb2total) {
      pb2 = 1;
    }
    prog = pb2;
  } 
  else if (bank == 3) {
    if (pb3 < pb2total) {
      pb3++;
    } else if (pb3 == pb3total) {
      pb3 = 1;
    }
    prog = pb3;
  }
  if (debug) {
    Serial.print("PROGRAM: ");
    Serial.println("right");
    Serial.println(buttonState);
  }
}

/**
   A function to handle the 'encoder' event without button
*/
void onEb1Encoder(EncoderButton& eb) {

  //displayUpdate();
  encoder_delta = eb.increment();
  long cstep = eb.increment() * 64;

  if (debug) {
    Serial.print("eb1 incremented by: ");
    Serial.println(eb.increment());
    Serial.print("eb1 position is: ");
    Serial.println(cstep);
  }
}
