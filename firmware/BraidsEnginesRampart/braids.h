#pragma once

#include "braids/envelope.h"
#include "braids/macro_oscillator.h"
#include "braids/quantizer.h"
#include "braids/signature_waveshaper.h"
#include "braids/quantizer_scales.h"
#include "braids/vco_jitter_source.h"

//const uint16_t decimation_factors[] = { 1, 2, 3, 4, 6, 12, 24 };
const uint16_t bit_reduction_masks[] = {
  0xffff,
  0xfff0,
  0xff00,
  0xf800,
  0xf000,
  0xe000,
  0xc000
};

#define     MI_SAMPLERATE      96000.f
#define     BLOCK_SIZE          32      // --> macro_oscillator.h !
#define     SAMP_SCALE          (float)(1.0 / 32756.0)


typedef struct
{
  braids::MacroOscillator *osc;
  float       samps[BLOCK_SIZE] ;
  int16_t     buffer[BLOCK_SIZE];
  uint8_t     sync_buffer[BLOCK_SIZE];

} PROCESS_CB_DATA ;

int16_t shared_buffer[BLOCK_SIZE];


//float a0 = (440.0 / 8.0) / kSampleRate; //48000.00;
const size_t   kBlockSize = BLOCK_SIZE;


struct Unit {
  braids::Quantizer   *quantizer;
  braids::SignatureWaveshaper *ws;
  braids::Envelope *envelope;
  bool            last_trig;
  // resampler
  //SRC_STATE       *src_state;
  PROCESS_CB_DATA pd;
  float           *samples;
  float           ratio;
};

static long src_input_callback(void *cb_data, float **audio);

struct Unit voices[1];

// initialize voice parameters calld at setup
void initVoices() {

  voices[0].ratio = 48000.f / MI_SAMPLERATE;

  // init some params
  voices[0].pd.osc = new braids::MacroOscillator;
  memset(voices[0].pd.osc, 0, sizeof(*voices[0].pd.osc));

  voices[0].pd.osc->Init(48000.f);
  voices[0].pd.osc->set_pitch((48 << 7));
  voices[0].pd.osc->set_shape(braids::MACRO_OSC_SHAPE_VOWEL_FOF);


  voices[0].ws = new braids::SignatureWaveshaper;
  voices[0].ws->Init(123774);

  voices[0].quantizer = new braids::Quantizer;
  voices[0].quantizer->Init();
  voices[0].quantizer->Configure(braids::scales[0]);

  //unit->jitter_source.Init();

  memset(voices[0].pd.buffer, 0, sizeof(int16_t)*BLOCK_SIZE);
  memset(voices[0].pd.sync_buffer, 0, sizeof(voices[0].pd.sync_buffer));
  memset(voices[0].pd.samps, 0, sizeof(float)*BLOCK_SIZE);

  voices[0].envelope = new braids::Envelope;
  voices[0].envelope->Init();

  voices[0].last_trig = false;
}


void updateBraidsAudio() {

  int16_t *buffer = voices[0].pd.buffer;
  uint8_t *sync_buffer = voices[0].pd.sync_buffer;
  size_t  size = BLOCK_SIZE;

  braids::MacroOscillator *osc = voices[0].pd.osc;

  //osc->set_pitch((pit << 7) + (int)(frac * 128.f));
  osc->set_pitch(pitch_in << 7);

  // cv modulation input
  float modulation;
  modulation = timb_mod; // timbre
  float timbre = (timbre_in + modulation);
  CONSTRAIN(timbre, 0.0f, 1.0f);

  modulation = morph_mod; // color
  float morph = (morph_in + modulation);
  CONSTRAIN(morph, 0.0f, 1.0f);

  // set parameters
  timbre = (timbre * 32767.0f);
  morph = (morph * 32767.0f); 

  osc->set_parameters(timbre, morph);

  // set shape/model
  uint8_t shape = (int)(engine_in);
  if (shape >= braids::MACRO_OSC_SHAPE_LAST)
    shape -= braids::MACRO_OSC_SHAPE_LAST;

  osc->set_shape(static_cast<braids::MacroOscillatorShape>(shape));

  bool trigger = (trigger_in != 0.0);
  bool trigger_flag = (trigger && (!voices[0].last_trig));

  voices[0].last_trig = trigger;


  if (trigger) {
    osc->Strike();
    env->gate(true);
    trigger_in = 0;
  } else { // just a fixed fake gate
    env->gate(false);
    //envTimer = 0;
  }

  for (int count = 0; count < 32; count += size) {
    // render
    osc->Render(sync_buffer, buffer, size);
  }

  // now apply the envelope
  /*for (size_t i = 0; i < 32; ++i) {
    int16_t sample =   (int16_t) ( (float) voices[0].pd.buffer[i] * env->process() ) ;
    shared_buffer[i] = sample;
  }*/
}
