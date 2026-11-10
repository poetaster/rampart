/*
  (c) 2026 blueprint@poetaster.de GPLv3

   Based on  mi_Ugens, Copyright (c) 2020 Volker Böhm. All rights reserved. GPLv3
   https://vboehm.net

   Mutable Instruments sources, including the stmlib, tides, plaits elements and braids libs are
   MIT License
   Copyright (c)  2020 (emilie.o.gillet@gmail.com)
*/

bool debug = true;

#include <Arduino.h>
#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include <hardware/pwm.h>

#include <PWMAudio.h>
#define SAMPLERATE 48000
#define PWMOUT 11
PWMAudio DAC(PWMOUT);  // 16 bit PWM audio

#include "Midier.h"

// button inputs
#define BUTTONL  5 // these are the touch surfaces
#define BUTTONR 6 

#include <MIDI.h>
#include <Wire.h>

//MIDI_CREATE_DEFAULT_INSTANCE();

// create ADSR envelope
#include "ADSR.h"
ADSR *env = new ADSR();
float envAttack;
float envDecay;
int envRelease;
float envSustain;
long envTimer = 0;

// braids dsp

#include <STMLIB.h>
#include <BRAIDS.h>

// globals used everywhere :)
float morph_in = 0.7f; // IN(4);
float trigger_in; //IN(5);
float level_in = 0.0f; //IN(6);
float harm_in = 0.1f;
float timbre_in = 0.1f;
int engine_in;

float fm_mod = 0.0f ; //IN(7);
float timb_mod = 0.0f; //IN(8);
float morph_mod = 0.0f; //IN(9);
float decay_mod = 0.5f; // IN(10);
float lpg_in = 0.1f ;// IN(11);
int pitch_in = 60;

float global_volume = 0.9;

#include "braids.h"

// END Braids

// GPIO, pots
const int POTA = 29; // set the analog input for fm_intensity
const int POTB = 28; // set the analog input for mod rate
const int POTC = 27; // set the analog input for mod ratio

// encoder related
#include <RotaryEncoder.h>
const int encoderA_pin = 10;
const int encoderB_pin = 9;
const int encoderSW_pin = 8;

RotaryEncoder encoder(encoderB_pin, encoderA_pin, RotaryEncoder::LatchMode::FOUR3);

void checkEncoderPosition() {
  encoder.tick();   // call tick() to check the state.
}

// display related
const int oled_sda_pin = 2;
const int oled_scl_pin = 3;
const int oled_i2c_addr = 0x3C;
const int dw = 128;
const int dh = 32;

#include <Adafruit_SSD1306.h>
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(dw, dh, &Wire1, OLED_RESET);

// TOF on same bus as adafruit

#include <VL53L1X.h>
VL53L1X sensor;

// the adc 4 inputs 16 bits
#include <AlashADS1115.h>
AlashADS1115 adc;

#include "font.h"
#include "helvnCB6pt7b.h"
#define myfont helvnCB6pt7b // Org_01 looks better but is small.
#include "shapes.h" // char array of names of instruments.
#include "display.h"


// variables for UI state management
int encoder_pos_last = 0;
int encoder_delta = 0;
uint32_t encoder_push_millis;
uint32_t step_push_millis;
bool encoder_held = false;

// buttons & knobs defines/functions
#include "control.h"
// midi related functions
#include "midi.h"

//utility
double randomDouble( double minf, double maxf) {
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}

float mapf(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  float result;
  result = (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
  return result;
}

int cv_avg = 3;
int16_t avg_cv(int cv_in) {

  //std::vector<int> data;
  int16_t val = 0;

  //for (int j = 0; j < cv_avg; ++j) data.push_back(analogRead(cv_in)); // val += analogRead(cv_in); // read the A/D a few times and average for a more stable value
  for (int j = 0; j < cv_avg; ++j) val += analogRead(cv_in); // read the A/D a few times and average for a more stable value
  val = val / cv_avg;

  //return median(data);

  return val;
}

void setup() {
  Serial.begin(115200);
  // this is the ADCs I2C
  Wire.setSDA(12);
  Wire.setSCL(13);
  Wire.begin();
  // start
  adc.begin();
  adc.setRate(860);
  adc.setAutoGain(true);
  
  // DISPLAY and TOF are on this bus
  Wire1.setSDA(oled_sda_pin);
  Wire1.setSCL(oled_scl_pin);
  Wire1.begin();
  Wire1.setClock(400000); // use 400 kHz I2Ca

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    //while (1);
  }


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.display();
  displaySplash();

  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);
  sensor.startContinuous(50);

  // set up Pico PWM audio output
  DAC.setBuffers(4, 32);  // DMA buffers
  //DAC.onTransmit(cb);
  DAC.setFrequency(SAMPLERATE);
  DAC.begin();

  // Additions
  // ENCODER
  pinMode(encoderA_pin, INPUT_PULLUP);
  pinMode(encoderB_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderA_pin), checkEncoderPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderB_pin), checkEncoderPosition, CHANGE);


  // let's get more resolution
  analogReadResolution(12);

  pinMode(BUTTONL, INPUT);
  pinMode(BUTTONR, INPUT);
  pinMode(encoderSW_pin, INPUT);

  pinMode(26, POTA);
  pinMode(27, POTC);
  pinMode(28, POTC);
  //pinMode(29, POTC);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(23, OUTPUT); // thi is to switch to PWM for power to avoid ripple noise
  digitalWrite(23, HIGH);

  // initialize enveloope settings
  env->setAttackRate(.01 * SAMPLERATE);  // .01 second
  env->setDecayRate(.1 * SAMPLERATE);
  env->setReleaseRate(3 * SAMPLERATE);
  env->setSustainLevel(.8);


  // init the braids voices
  initVoices();
  // initialize a mode to play
  mode = midier::Mode::Ionian;
  makeScale( roots[scaleRoot], mode);

  // Connect the HandleNoteOn function to the library, so it is called upon reception of a NoteOn.
  //MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
  //MIDI.setHandleNoteOff(HandleNoteOff);  // Put only the name of the function
  //MIDI.begin(MIDI_CHANNEL_OMNI);

  delay(2000);

}


void updateControl() {

  //MIDI.read();

  uint32_t now = millis();
  bool anybuttonpressed;
  anybuttonpressed = false;

  int p1 = potvalue[0]; //analogRead(POTA); // value is 0-4065
  int p2 = potvalue[1]; // analogRead(POTC); // value is 0-4065


  morph_in = (float)p1 / 4095.0f; //map(p1, 0, 4065, 0.0, 1.0); // IN(2);
  timbre_in = (float)p2 / 4095.0f; //map(p2, 0, 4065, 0.0, 1.0); //IN(3);
  CONSTRAIN(morph_in, 0.0f, 1.0f);
  CONSTRAIN(timbre_in, 0.0f, 1.0f);

  // current TOF distance measure
  sensor.read();

  int currentRange = map(sensor.ranging_data.range_mm, 10, 1000, 21, 0); // distance from tofaa
  if (currentRange > 0) {
    if (debug) Serial.print("range: ");
    if (debug) Serial.println(currentRange);
    int newNote = currentMode[currentRange];
    if (newNote != pitch_in && newNote < 121) {
      aNoteOff(pitch_in, 0); // second param velocity
      pitch_in = newNote;
      trigger_in = 1.0;
      if (debug) Serial.print("note: ");
      if (debug) Serial.println(pitch_in);
      //aNoteOn( pitch_in, 100); // don't need this
    }
  }


  if (BUTTONL) { }

  if (digitalRead(BUTTONR)== 1) {
    modeIndex = modeIndex + 1;
    if (modeIndex == 8 ) modeIndex = 0;
    if (debug) Serial.print("mode: ");
    if (debug) Serial.println(modeIndex);
    switch (modeIndex) {
      case 0:
        mode = midier::Mode::Ionian;
        break;
      case 1:
        mode = midier::Mode::Dorian;
        break;
      case 2:
        mode = midier::Mode::Phrygian;
        break;
      case 3:
        mode = midier::Mode::Lydian;
        break;
      case 4:
        mode = midier::Mode::Mixolydian;
        break;
      case 5:
        mode = midier::Mode::Aeolian;
        break;
      case 6:
        mode = midier::Mode::Locrian;
        break;
    }
    makeScale( roots[scaleRoot], mode);
  }

  // finally, collect adc inputs
  fm_mod =   (float)adc.voltage(0)/7.0f; // 7 is at a 1.2k to ground
  timb_mod = (float)adc.voltage(1)/7.0f;
  morph_mod = (float)adc.voltage(2)/7.0f;
  decay_mod = (float)adc.voltage(3)/7.0f;
}

void loop() {
  // when the osc buffer has been written to PWM buffer
    if ( DAC.availableForWrite()) {
      updateBraidsAudio();
        for (size_t i = 0; i < 32; i++) {
          int16_t sample =   (int16_t) ( (float) voices[0].pd.buffer[i] * env->process() * global_volume  ) ;
          DAC.write( sample );
          DAC.write( sample );
        }
    }
}

// second core dedicated to display foo

void setup1() {
  delay (1000); // wait for main core to start up perhipherals
}

int engineCount = 0;
int engineInc = 0;

// second core deals with ui / control rate updates
void loop1() {

  updateControl(); //pots values on this loop since the other delays

  uint32_t now = millis();
  bool anybuttonpressed;
  anybuttonpressed = false;

  // UI handlers
  // first encoder
  encoder.tick();

  int encoder_pos = encoder.getPosition();
  if ( (encoder_pos != encoder_pos_last )) {
    encoder_delta = encoder_pos - encoder_pos_last;
  }

  if ( encoder_pos != encoder_pos_last  ) {
    engineCount = engineCount + encoder_delta;
    CONSTRAIN(engineCount, 0, 47);
    engine_in = engineCount;

  }


  // now, after buttons check if only encoder moved and no buttons
  // this is broken by mozzi, sigh.
  if (! anybuttonpressed && encoder_delta) {
    //float turn = encoder_delta * 0.01f;
    //harm_in = harm_in + turn;

  }


  /// only set new pos last after buttons have had a chance to use the delta
  encoder_pos_last = encoder_pos;
  encoder_delta = 0;  // we've used it

  // start tracking time encoder button held
  if ( ! digitalRead( encoderSW_pin ) ) {
    encoder_push_millis = now;
  } else {
    encoder_push_millis = 0;
    encoder_held = false;
  }

  // lock pot settings when no keys are pressed so it requires more movement to change value
  // this is so when we change tracks we don't immediately change the settings on the new track
  if (!anybuttonpressed) lockpots();

  // reading A/D seems to cause noise in the audio so don't do it too often
  if ((now - pot_timer) > POT_SAMPLE_TIME) {
    readpot(0);
    readpot(1);
    readpot(3);
    pot_timer = now;
  }

  displayBraids();

}
