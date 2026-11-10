// display functions/defines

#define DISPLAY_TIME 2000 // time in ms to display numbers on LEDS

enum {
  MODE_PLAY = 0,
  MODE_CONFIG,
  MODE_COUNT   // how many modes we got
};

int display_mode = 0;
uint8_t display_repeats = 0;

int32_t display_timer;

// display functions


typedef struct {
  int x;
  int y;
  const char* str;
} pos_t;

/* draw a circle gauge 8 pixels line in a half circle.
    int data curren value, 1 - 100
    int centerX, centerY, placement on screen assuming 128x32
*/
void drawCircle( int data, int centerX = 64, int centerY = 16) {
  int pointerLength = 6;
  int angle = map(data, 0, 100, -180, 0); // Map the data to an angle
  float radian = DEG_TO_RAD * angle; // Convert the angle to radian
  int x = round(centerX + (pointerLength * cos(radian)));
  int y = round(centerY + (pointerLength * sin(radian)));
  //drawLine(centerX, centerY, x, y);
  display.drawLine(centerX, centerY, x, y, SSD1306_WHITE);
  //display.display(); // Update screen with each newly-drawn
}

//// {x,y} locations of play screen items
const int step_text_pos[] = { 0, 15, 16, 15, 32, 15, 48, 15, 64, 15, 80, 15, 96, 15, 112, 15 };

const pos_t line_1_1    = {.x = 0,  .y = 8, .str = "bpm:%3d" };
const pos_t line_1_2  = {.x = 36, .y = 8, .str = "trs:%+2d" };
const pos_t line_1_3  = {.x = 100, .y = 8, .str = "seq:%d" };
const pos_t line_2_1    = {.x = 0,  .y = 22, .str = "" };
const pos_t line_2_2   = {.x = 36, .y = 22, .str = "" };
const pos_t line_2_3   = {.x = 72, .y = 22, .str = "" };
const pos_t line_3_1 = { .x = 0, .y = 27,  .str = "" };
const pos_t line_3_2 = { .x = 36, .y = 27, .str = "" };
const pos_t line_3_3 = { .x = 100, .y = 27,  .str = "" };

const pos_t oct_text_offset = { .x = 3, .y = 10,  .str = "" };
const pos_t gate_bar_offset = { .x = 0, .y = -15, .str = "" };
const pos_t edit_text_offset = { .x = 3, .y = 22,  .str = "" };

const int gate_bar_width = 14;
const int gate_bar_height = 4;

void updateGauges() {
  drawCircle( global_volume * 100, line_1_3.x + 16, line_1_3.y ); // global volume
  drawCircle( morph_in * 100, line_2_1.x + 16, line_2_1.y ); // morph
                                                             // 
  /*if (voice_number != 2) {
    drawCircle( harm_in * 100, line_2_2.x + 16, line_2_2.y ); // harm
    drawCircle( position_in * 100, line_3_3.x + 16, line_3_3.y ); // position
  }*/
  drawCircle( timbre_in * 100, line_2_3.x + 16, line_2_3.y ); //  timbre
}

void displayADSR() {
  display.clearDisplay();
  display.setFont(&Org_01);
  // // name
  display.setCursor(line_1_1.x, line_1_1.y);
  display.print("ADSR");
  // volume
  display.setCursor(line_1_3.x, line_1_3.y);
  display.print("V");
  drawCircle( global_volume * 100, line_1_3.x + 16, line_1_3.y ); // global volume

  // attack
  display.setCursor(line_2_1.x, line_2_1.y);
  display.print("A");
  drawCircle( envAttack * 100, line_2_1.x + 16, line_2_1.y ); 

  // decay
  display.setCursor(line_2_2.x, line_2_2.y);
  display.print("D");
  drawCircle( envDecay * 100, line_2_2.x + 16, line_2_2.y ); // harm

  // sustain
  display.setCursor(line_2_3.x, line_2_3.y);
  display.print("S");
  drawCircle( envSustain * 100, line_2_3.x + 16, line_2_3.y ); //  timbre
  
  // release
  display.setCursor(line_3_3.x, line_3_3.y);
  display.print("R: ");
  display.print(envRelease);

  display.display();
}
/* braids */
void displayBraids() {
  display.clearDisplay();
  display.setFont(&myfont);
  // // name
  display.setCursor(line_1_1.x, line_1_1.y);
  display.print(oscnames[engine_in]);

  // volume
  display.setCursor(line_1_3.x, line_1_3.y);
  display.print("V");

  display.setCursor(line_2_1.x, line_2_1.y);
  display.print("C");
  //display.print(morph_in);
  // harmonics
  //display.setCursor(line_2_2.x, line_2_2.y);
  //display.print("H: ");
  //display.print(harm_in);  // user sees 1-8
  // timber
  display.setCursor(line_2_3.x, line_2_3.y);
  display.print("T");

  //display.print(timbre_in);
  //display.setCursor(line_1_2.x, line_2_1.y);
  //display.print("P:");
  //display.print(voices[0].patch.note);
  // play/pause
  //display.setCursor(line_2_3.x, line_2_3.y);
  //display.print("m: ");
  //display.print(display_mode);

  updateGauges();
  display.display();
}

void displaySplash() {
  display.clearDisplay();
  display.setFont(&myfont);
  display.setTextColor(WHITE, 0);
  display.drawRect(0, 0, dw - 1, dh - 1, WHITE);
  display.setCursor(10, 16);
  display.print("Ramparts' Braid");
  display.display();
}
