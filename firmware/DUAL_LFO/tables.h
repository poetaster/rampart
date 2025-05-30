#include "wave/chum78_int8.h"
#include "wave/chum9_int8.h"
#include "wave/cos512_int8.h"
#include "wave/cosphase256_int8.h"
#include "wave/phasor256_int8.h"
#include "wave/saw256_int8.h"
#include "wave/saw_analogue512_int8.h"
#include "wave/sin512_int8.h"
#include "wave/square_analogue512_int8.h"
#include "wave/triangle512_int8.h"
#include "wave/triangle_analogue512_int8.h"
//#include "wave/waveshape1_softclip_int8.h"
#include "wave/waveshape2_softerclip_int8.h"
#include "wave/waveshape_chebyshev_3rd_256_int8.h"
#include "wave/waveshape_chebyshev_4th_256_int8.h"
#include "wave/waveshape_chebyshev_5th_256_int8.h"
//#include "wave/waveshape_chebyshev_6th_256_int8.h"
#include "wave/waveshape_sigmoid_int8.h"
//#include "wave/waveshape_tanh_int8.h"
// Waves original
#include "wave/noise256.h"
#include "wave/ramp256.h"
#include "wave/saw256.h"
#include "wave/sine256.h"
#include "wave/tri256.h"
#include "wave/pulse8.h"
#include "wave/pulse16.h"
#include "wave/pulse64.h"
#include "wave/sq256.h"

// first line from the original sketch
const int8_t *waveTables[] = { 
sine256, ramp256, saw256, tri256, pulse8, pulse16, pulse64, sq256, noise256,
SAW_ANALOGUE512_DATA,//keep
SQUARE_ANALOGUE512_DATA,
TRIANGLE_ANALOGUE512_DATA,
//WAVESHAPE1_SOFTCLIP_DATA,
WAVESHAPE2_SOFTERCLIP_DATA,
CHUM78_DATA, //???
CHUM9_DATA, // keep
COS512_DATA, // keep
COSPHASE256_DATA,//keep
PHASOR256_DATA, // keep
//SAW256_DATA,
//SIN512_DATA,// keep
//TRIANGLE512_DATA,
CHEBYSHEV_3RD_256_DATA,
CHEBYSHEV_4TH_256_DATA,
CHEBYSHEV_5TH_256_DATA,
//CHEBYSHEV_6TH_256_DATA,
WAVESHAPE_SIGMOID_DATA,
};
