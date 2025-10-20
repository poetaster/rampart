/**
   @file MultiModeEnv.ino
   @author Adam Wonak (https://github.com/awonak/)
   @author Hagiwo (https://note.com/solder_state/n/n69643b792274)
   @brief Multi-mode Envelope Generator firmware for HAGIWO Sync Mod LFO (demo: TODO)
   @version 0.3
   @date 2023-04-19

   @copyright Copyright (c) 2023

*/

// Envelope curve wavetable
const static word LOG[200] PROGMEM = {
  0, 64, 93, 122, 150, 177, 203, 228, 253, 277, 299, 322, 343, 364, 384, 404, 423, 441, 459, 476, 493, 509, 524, 539, 554, 568, 582, 595, 608, 620, 632, 644, 655, 666, 677, 687, 697, 707, 716, 725, 734, 742, 751, 759, 766, 774, 781, 788, 795, 801, 808, 814, 820, 825, 831, 836, 842, 847, 852, 856, 861, 865, 870, 874, 878, 882, 886, 889, 893, 896, 900, 903, 906, 909, 912, 915, 917, 920, 923, 925, 928, 930, 932, 934, 937, 939, 941, 942, 944, 946, 948, 950, 951, 953, 954, 956, 957, 959, 960, 961, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 978, 979, 980, 980, 981, 982, 982, 983, 984, 984, 985, 985, 986, 986, 987, 987, 988, 988, 988, 989, 989, 990, 990, 990, 991, 991, 991, 992, 992, 992, 993, 993, 993, 993, 994, 994, 994, 994, 995, 995, 995, 995, 995, 996, 996, 996, 996, 996, 997, 997, 997, 997, 997, 997, 997, 998, 998, 998, 998, 998, 998, 998, 998, 998, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 1000, 1000, 1000, 1000, 1000, 1000, 1000
};
const static word LINEAR[200] PROGMEM = {
  0, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235, 240, 245, 250, 255, 260, 265, 270, 275, 280, 285, 290, 295, 300, 305, 310, 315, 320, 325, 330, 335, 340, 345, 350, 355, 360, 365, 370, 375, 380, 385, 390, 395, 400, 405, 410, 415, 420, 425, 430, 435, 440, 445, 450, 455, 460, 465, 470, 475, 480, 485, 490, 495, 500, 505, 510, 515, 520, 525, 530, 535, 540, 545, 550, 555, 560, 565, 570, 575, 580, 585, 590, 595, 600, 605, 610, 615, 620, 625, 630, 635, 640, 645, 650, 655, 660, 665, 670, 675, 680, 685, 690, 695, 700, 705, 710, 715, 720, 725, 730, 735, 740, 745, 750, 755, 760, 765, 770, 775, 780, 785, 790, 795, 800, 805, 810, 815, 820, 825, 830, 835, 840, 845, 850, 855, 860, 865, 870, 875, 880, 885, 890, 895, 900, 905, 910, 915, 920, 925, 930, 935, 940, 945, 950, 955, 960, 965, 970, 975, 980, 985, 990, 995, 1000
};
const static word EXP[200] PROGMEM = {
  0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 12, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 18, 18, 19, 20, 20, 21, 22, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 39, 40, 41, 43, 44, 46, 47, 49, 50, 52, 54, 56, 58, 59, 61, 63, 66, 68, 70, 72, 75, 77, 80, 83, 85, 88, 91, 94, 97, 100, 104, 107, 111, 114, 118, 122, 126, 130, 135, 139, 144, 148, 153, 158, 164, 169, 175, 180, 186, 192, 199, 205, 212, 219, 226, 234, 241, 249, 258, 266, 275, 284, 293, 303, 313, 323, 334, 345, 356, 368, 380, 392, 405, 418, 432, 446, 461, 476, 491, 507, 524, 541, 559, 577, 596, 616, 636, 657, 678, 701, 723, 747, 772, 797, 823, 850, 878, 907, 936, 967, 1000
};

int i = 0;  // Envelope progress state time count

long time = 100;      // Envelope delay time between incremental change
bool attack_end = 0;  // State of rising or falling envelope: 0=rising, 1=falling
float duty = 0;       // PWM duty

int current_mm_mode = 0;
int current_mm_curve = 0;

enum Mode {
  MODE_AR,
  MODE_ASR,
  MODE_SLOW,
  MODE_LFO,
};

enum Curve {
  CURVE_LOG,
  CURVE_LINEAR,
  CURVE_EXP,
};

Mode read_mode() {
  switch (current_mm_mode) {
    case 0:
      return MODE_AR;
    case 1:
      return MODE_ASR;
    case 2:
      return MODE_SLOW;
    case 3:
      return MODE_LFO;
  }
}

Curve read_curve() {
  switch (current_mm_curve) {
    case 0:
      return CURVE_LOG;
    case 1:
      return CURVE_LINEAR;
    case 2:
      return CURVE_EXP;
  }
}

void loopMMenv() {
  // Read cv inputs to determine state for this loop.
  hw.ProcessInputs();

  bool gate_rising = hw.gate.State() == DigitalInput::STATE_RISING;
  bool gate_high = hw.gate.On();
  Mode mode = read_mode();

  // Detect if a new trigger has been received.
  if (gate_rising) {
    duty = 0;
    attack_end = 0;
    i = 0;
  }

  // Update rising / falling state.
  // Rising envelope
  if (attack_end == 0) {
    // At minimum attack levels, traverse the curve at a faster rate than default.
    (hw.p1.Read() > 0) ? i++ : i += min(199 - i, 10);

    if (i >= 199) {
      attack_end = 1;
    }
  }
  // Falling envelope
  else if (attack_end == 1) {
    // Check if falling and not in sustain mode.
    if (i > 0 && !(mode == MODE_ASR && gate_high)) {
      // At minimum release levels, traverse the curve at a faster rate than default.
      (hw.p2.Read() > 0) ? i-- : i -= min(i, 10);
    }
    // If looping mode, restart rising
    if (mode == MODE_LFO && i == 0) {
      attack_end = 0;
    }
  }

  // Set envelope cv output according to curve choice.
  switch (read_curve()) {
    case CURVE_LOG:
      duty = (pgm_read_word(&(LOG[i])));
      break;

    case CURVE_LINEAR:
      duty = (pgm_read_word(&(LINEAR[i])));
      break;

    case CURVE_EXP:
      duty = (pgm_read_word(&(EXP[i])));
      break;
  }

  // Attack / Release incremental delay time (use Millisecond delay for SLOW AR mode).
  time = (attack_end == 0)
         ? hw.p1.Read()   // Attack knob
         : hw.p2.Read();  // Release knob

  // Short sleep duration before advancing to the next step in the curve table.
  (mode == MODE_SLOW)
  ? delay(time / 10)               // Slower milliscond delay for SLOW mode.
  : delayMicroseconds(time * 10);  // Faster microsecond delay by default.

  // Write CV Output (scaled down to PWM range of 0 to 255)
  hw.output.Update(map(duty, 0, 1000, 0, 255));
}
