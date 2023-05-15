#define B2_PIN 2

// constants won't change
const int RELAY_PIN = 6;  // the Arduino pin, which connects to the IN pin of relay


// variables will change: 1-3
int buttonState = 0;


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin as an output.
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(B2_PIN, INPUT);
}

void updateControl() {

  static int previous2;
  int current2 = digitalRead(B2_PIN);
  if (previous2 == LOW && current2 == HIGH) {
    if (buttonState == 1) {
      buttonState = 0;
    } else {
      buttonState = 1;
    }
  }
  previous2 = current2;
  if ( buttonState == 0 ) {
    digitalWrite(RELAY_PIN,LOW);
  } else if ( buttonState == 1 ) {
     digitalWrite(RELAY_PIN,HIGH);
  }
  if (buttonState == 1) {
    // turn LED on:
    digitalWrite(13, HIGH);
  } else {
    // turn LED off:
    digitalWrite(13, LOW);
  }

}



// the loop function runs over and over again forever
void loop() {
 /* digitalWrite(RELAY_PIN, HIGH);
  delay(100);
  digitalWrite(RELAY_PIN, LOW);
  delay(100);*/
  updateControl();
  //delay(1000);
}
