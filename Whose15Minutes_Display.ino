// Whose 15 Minutes?: game instructions on a VGA screen
// Library: simple-circuit.com Arduino VGA library (VGA.h / VGA.cpp in this folder)
//          + Adafruit GFX Library (install with the Library Manager)
// Screen: 160 x 64 pixels, black and white
//
// Wiring on Arduino UNO:
//   pin 3  -> HSYNC (VGA pin 13)
//   pin 10 -> VSYNC (VGA pin 14)
//   pin 6  -> 220 ohm -> VGA pins 1, 2, 3 together
//   GND    -> VGA pins 5-8 and 10
//
// Text screens stay until a button is pressed.
// Buttons (each between its pin and GND, no resistor, internal pull-up):
//   pin 8  = NEXT : next screen at once (also ends the 5 minute round)
//   pin 12 = BACK : previous screen
//   both together = RESET : back to the first screen
//   A single press acts when the button is released, so a press of both
//   buttons is not read as NEXT or BACK first. They need not go down at
//   exactly the same moment: hold one, then press the other.
//   Do not use pins 0-7 for buttons: the VGA code writes all of PORTD for
//   every pixel, so their internal pull-up does not stay on (pin 5 failed).
//   Free pins for more buttons: 8, 9, 11, 12, A0-A5.
// Round screens count down 5 minutes, then the game goes on.
// On a round screen, both buttons restart the timer.
//
// Layout of a text screen:
//   title (size 2) rows 0-13, icon 14x14 top right, line at row 18,
//   body (size 1) 3 lines at rows 23, 32, 41, max 26 characters,
//   footer at row 56: step counter
// Do not use delay() or millis(). Use display.delay().

#include <Adafruit_GFX.h>
#include "VGA.h"

VGA display = VGA();

const uint16_t ROUND_SECONDS = 300;    // length of one round (5 minutes)
const byte     PIN_NEXT      = 8;      // NEXT button to GND (active LOW, internal pull-up)
const byte     PIN_BACK      = 12;     // BACK button to GND (active LOW, internal pull-up)


// Buttons that work on a screen
const byte K_BACK  = 1;
const byte K_NEXT  = 2;
const byte K_START = 4;
const byte K_NEW   = 8;    // START begins a new game
const byte K_TIMER = 16;   // round with timer

const byte SCREEN_COUNT = 34;
byte screen = 0;
byte keys = 0;

typedef const __FlashStringHelper* FS;

// ---------- icons (14 x 14 pixels) ----------

enum Icon : byte {
  IC_NONE,
  IC_BULB,
  IC_LIST,
  IC_CITY,
  IC_CARD,
  IC_TARGET,
  IC_PLUG,
  IC_PERSON,
  IC_PEOPLE,
  IC_BUS,
  IC_CLOCK,
  IC_BARS,
  IC_SPEECH,
  IC_FLAG,
  IC_BOOK
};

const uint8_t ICONS[14][28] PROGMEM = {
  {0x07, 0x80, 0x18, 0x60, 0x20, 0x10, 0x40, 0x08, 0x40, 0x08, 0x40, 0x08, 0x20, 0x10, 0x10, 0x20, 0x08, 0x40, 0x0F, 0xC0, 0x0F, 0xC0, 0x0F, 0xC0, 0x07, 0x80, 0x00, 0x00},  // BULB
  {0x00, 0x00, 0xDF, 0xF8, 0xDF, 0xF8, 0x00, 0x00, 0xDF, 0xF8, 0xDF, 0xF8, 0x00, 0x00, 0xDF, 0xF8, 0xDF, 0xF8, 0x00, 0x00, 0xDF, 0xF8, 0xDF, 0xF8, 0x00, 0x00, 0x00, 0x00},  // LIST
  {0x00, 0xF8, 0x00, 0x88, 0x7C, 0xA8, 0x44, 0x88, 0x54, 0xA8, 0x44, 0x88, 0x54, 0xA8, 0x44, 0x88, 0x54, 0xA8, 0x44, 0x88, 0x44, 0x88, 0x44, 0x88, 0xFF, 0xFC, 0x00, 0x00},  // CITY
  {0x00, 0x00, 0x7F, 0xF8, 0x40, 0x08, 0x5C, 0x08, 0x5D, 0xE8, 0x5C, 0x08, 0x41, 0xE8, 0x40, 0x08, 0x5F, 0xE8, 0x40, 0x08, 0x5F, 0x88, 0x40, 0x08, 0x7F, 0xF8, 0x00, 0x00},  // CARD
  {0x0F, 0xC0, 0x30, 0x30, 0x40, 0x08, 0x47, 0x88, 0x88, 0x44, 0x90, 0x24, 0x93, 0x24, 0x93, 0x24, 0x90, 0x24, 0x88, 0x44, 0x47, 0x88, 0x40, 0x08, 0x30, 0x30, 0x0F, 0xC0},  // TARGET
  {0x10, 0x80, 0x10, 0x80, 0x10, 0x80, 0x7F, 0xE0, 0x40, 0x20, 0x40, 0x20, 0x20, 0x40, 0x1F, 0x80, 0x06, 0x00, 0x06, 0x00, 0x03, 0x00, 0x01, 0xC0, 0x00, 0x30, 0x00, 0x0C},  // PLUG
  {0x07, 0x80, 0x0F, 0xC0, 0x0F, 0xC0, 0x07, 0x80, 0x00, 0x00, 0x1F, 0xE0, 0x3F, 0xF0, 0x2F, 0xD0, 0x2F, 0xD0, 0x2F, 0xD0, 0x0C, 0xC0, 0x0C, 0xC0, 0x0C, 0xC0, 0x0C, 0xC0},  // PERSON
  {0x30, 0x30, 0x78, 0x78, 0x78, 0x78, 0x30, 0x30, 0x00, 0x00, 0x78, 0x78, 0xFC, 0xFC, 0xFC, 0xFC, 0xB4, 0xB4, 0xB4, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30},  // PEOPLE
  {0x00, 0x00, 0x7F, 0xF8, 0x40, 0x08, 0x5B, 0x68, 0x5B, 0x68, 0x40, 0x08, 0x40, 0x08, 0x7F, 0xF8, 0x40, 0x08, 0x58, 0x68, 0x7F, 0xF8, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00},  // BUS
  {0x0F, 0xC0, 0x30, 0x30, 0x41, 0x08, 0x41, 0x08, 0x81, 0x04, 0x81, 0x04, 0x81, 0xE4, 0x80, 0x04, 0x80, 0x04, 0x40, 0x08, 0x40, 0x08, 0x30, 0x30, 0x0F, 0xC0, 0x00, 0x00},  // CLOCK
  {0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x03, 0x30, 0x03, 0x30, 0x03, 0x30, 0x33, 0x30, 0x33, 0x30, 0x33, 0x30, 0x33, 0x30, 0x33, 0x30, 0x33, 0x30, 0xFF, 0xFC, 0x00, 0x00},  // BARS
  {0x00, 0x00, 0x7F, 0xF8, 0x80, 0x04, 0x9F, 0xE4, 0x80, 0x04, 0x9F, 0x84, 0x80, 0x04, 0x7F, 0xF8, 0x18, 0x00, 0x10, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // SPEECH
  {0x40, 0x00, 0x7F, 0xE0, 0x7F, 0xC0, 0x7F, 0x80, 0x7F, 0xC0, 0x7F, 0xE0, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0xE0, 0x00},  // FLAG
  {0x00, 0x00, 0x7C, 0xF8, 0x83, 0x04, 0xBB, 0x74, 0x83, 0x04, 0xBB, 0x74, 0x83, 0x04, 0xBB, 0x74, 0x83, 0x04, 0x83, 0x04, 0x7C, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // BOOK
};

void drawIcon(byte id, int x, int y) {
  if (id) display.drawBitmap(x, y, ICONS[id - 1], 14, 14, WHITE);
}

// ---------- drawing ----------

int textWidth(FS s, byte size) {
  return strlen_P((PGM_P)s) * 6 * size - size;
}

void header(FS title, byte icon) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(title);
  display.setTextSize(1);
  drawIcon(icon, VGAWIDTH - 14, 0);
  display.drawFastHLine(0, 16, VGAWIDTH, WHITE);
}

void footer(byte k, FS step) {
  if (k & K_BACK) {
    display.setCursor(0, 57);
    display.print(F("< BACK"));
  }
  if (step) {
    display.setCursor((VGAWIDTH - textWidth(step, 1)) / 2, 57);
    display.print(step);
  }
  if (k & (K_NEXT | K_START | K_NEW)) {
    FS right = F("NEXT >");
    display.setCursor(VGAWIDTH - textWidth(right, 1), 57);
    display.print(right);
  }
}

void textScreen(FS title, byte icon, FS step, FS l1, FS l2, FS l3, byte k) {
  keys = k;
  header(title, icon);
  FS lines[3] = {l1, l2, l3};
  for (byte i = 0; i < 3; i++) {         // body rows 25, 34, 43: one empty row
    if (lines[i]) {                      // below the title line and above the footer
      display.setCursor(0, 25 + i * 9);
      display.print(lines[i]);
    }
  }
  footer(k, step);
}

void agendaScreen() {
  keys = K_BACK | K_NEXT;
  header(F("AGENDA"), IC_LIST);
  const byte icons[5] = {IC_BOOK, IC_PERSON, IC_BARS, IC_PEOPLE, IC_SPEECH};
  FS labels[5] = {F("Rules"), F("7/3"), F("Talk"), F("5/5"), F("Ask")};
  for (byte i = 0; i < 5; i++) {
    int x = i * 32;
    drawIcon(icons[i], x + 9, 23);
    display.setCursor(x + (32 - textWidth(labels[i], 1)) / 2, 41);
    display.print(labels[i]);
    if (i < 4) {
      display.setCursor(x + 29, 27);
      display.print('>');
    }
  }
  footer(keys, NULL);
}

void timerScreen(FS title, FS sub) {
  keys = K_START | K_TIMER;
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(title);
  display.setCursor(VGAWIDTH - textWidth(sub, 1), 0);
  display.print(sub);
  display.drawFastHLine(0, 10, VGAWIDTH, WHITE);
  FS foot = F("NEXT=done BOTH=restart");
  display.setCursor((VGAWIDTH - textWidth(foot, 1)) / 2, 57);
  display.print(foot);
}

void drawTime(uint16_t secs, bool show) {
  display.fillRect(0, 20, VGAWIDTH, 28, BLACK);
  if (!show) return;
  char buf[5];
  buf[0] = '0' + secs / 60;
  buf[1] = ':';
  buf[2] = '0' + (secs % 60) / 10;
  buf[3] = '0' + secs % 10;
  buf[4] = 0;
  display.setTextSize(3);
  display.setCursor((VGAWIDTH - (4 * 18 - 3)) / 2, 22);
  display.print(buf);
  display.setTextSize(1);
}

// ---------- screens ----------

void showScreen(byte n) {
  switch (n) {
    case 0: textScreen(F("15 MINUTES"), IC_CLOCK, F("1/2"), F("Welcome to the 15-minute"), F("city! Everything should"), F("be a short walk from home."), K_NEXT); break;
    case 1: textScreen(F("15 MINUTES"), IC_CLOCK, F("2/2"), F("The idea: work, shops,"), F("health and school are all"), F("15 minutes away on foot."), K_BACK | K_NEXT); break;
    case 2: textScreen(F("PLAYERS"), IC_PEOPLE, F("1/2"), F("At least in theory."), F("Two players share the"), F("flat in the middle."), K_BACK | K_NEXT); break;
    case 3: textScreen(F("PLAYERS"), IC_PEOPLE, F("2/2"), F("RED is the care persona"), F("(player 1). BLUE is the"), F("earner persona (player 2)."), K_BACK | K_NEXT); break;
    case 4: textScreen(F("HOW TO PLAY"), IC_LIST, NULL, F("The game has two rounds"), F("of 5 minutes. Then you"), F("talk about the result."), K_BACK | K_NEXT); break;
    case 5: textScreen(F("THE CITY"), IC_CITY, F("1/2"), F("Each street corner has a"), F("breadboard. Home is the"), F("flat at the centre corner."), K_BACK | K_NEXT); break;
    case 6: textScreen(F("THE CITY"), IC_CITY, F("2/2"), F("Ten buildings have an LED."), F("It turns on when your"), F("wires reach it, no gaps."), K_BACK | K_NEXT); break;
    case 7: textScreen(F("THE WIRES"), IC_PLUG, F("1/3"), F("One wire bridges one"), F("street, from one corner"), F("to the next corner."), K_BACK | K_NEXT); break;
    case 8: textScreen(F("THE WIRES"), IC_PLUG, F("2/3"), F("Start at home. Lay your"), F("wires along the streets"), F("from corner to corner."), K_BACK | K_NEXT); break;
    case 9: textScreen(F("THE WIRES"), IC_PLUG, F("3/3"), F("Plug each wire end into"), F("the breadboard at the"), F("corner. No gaps allowed."), K_BACK | K_NEXT); break;
    case 10: textScreen(F("THE GOAL"), IC_TARGET, F("1/2"), F("Reach all your"), F("destinations within your"), F("wire budget and the time."), K_BACK | K_NEXT); break;
    case 11: textScreen(F("THE GOAL"), IC_TARGET, F("2/2"), F("Connect only your own"), F("destinations. An LED does"), F("not know whose wire it is."), K_BACK | K_NEXT); break;
    case 12: textScreen(F("ROUND 1"), IC_PERSON, F("1/8"), F("Each destination on the"), F("board is marked RED or"), F("BLUE. Find your colour."), K_BACK | K_NEXT); break;
    case 13: textScreen(F("ROUND 1"), IC_PERSON, F("2/8"), F("RED has 12 wires."), F("Destinations:"), F("part-time job, daycare,"), K_BACK | K_NEXT); break;
    case 14: textScreen(F("ROUND 1"), IC_PERSON, F("3/8"), F("paediatrician, pharmacy,"), F("supermarket, post office,"), F("parents' flat."), K_BACK | K_NEXT); break;
    case 15: textScreen(F("ROUND 1"), IC_PERSON, F("4/8"), F("BLUE has 13 wires."), F("Destinations: full-time"), F("job, gym, hardware store."), K_BACK | K_NEXT); break;
    case 16: textScreen(F("WHY 12/13?"), IC_PEOPLE, F("5/8"), F("RED walks with the child"), F("and carries bags, so RED"), F("gets one wire less."), K_BACK | K_NEXT); break;
    case 17: textScreen(F("ROUND 1"), IC_PERSON, F("6/8"), F("Both start at home. Each"), F("player connects only"), F("their own destinations."), K_BACK | K_NEXT); break;
    case 18: textScreen(F("ROUND 1"), IC_PERSON, F("7/8"), F("You do not need to walk"), F("back home. Go on from each"), F("destination you reach."), K_BACK | K_NEXT); break;
    case 19: textScreen(F("ATTENTION"), IC_TARGET, F("8/8"), F("RED goes to work first,"), F("then to the daycare and"), F("then to the paediatrician."), K_BACK | K_NEXT); break;
    case 20: textScreen(F("ROUND 1"), IC_PERSON, NULL, F("You have 5 minutes."), F("Press NEXT to start the"), F("timer."), K_BACK | K_NEXT); break;
    case 21: timerScreen(F("ROUND 1"), F("5 minutes")); break;
    case 22: textScreen(F("TIME UP"), IC_CLOCK, NULL, F("Stop and leave all wires"), F("in place. Count the LEDs"), F("that are on."), K_NEXT); break;
    case 23: textScreen(F("THE BIAS"), IC_BARS, F("1/4"), F("The 15-minute city plans"), F("single trips from home."), F("Care work is left out."), K_BACK | K_NEXT); break;
    case 24: textScreen(F("THE BIAS"), IC_BARS, F("2/4"), F("A care day links many"), F("stops with fixed times."), F("Close stops, long day."), K_BACK | K_NEXT); break;
    case 25: textScreen(F("THE BIAS"), IC_BARS, F("3/4"), F("Women make 58% of all"), F("care trips. Care is almost"), F("a third of their mobility."), K_BACK | K_NEXT); break;
    case 26: textScreen(F("THE BIAS"), IC_BARS, F("4/4"), F("So the 15-minute city"), F("fits men's days better"), F("than women's days."), K_BACK | K_NEXT); break;
    case 27: textScreen(F("ROUND 2"), IC_PEOPLE, F("1/2"), F("Pull out all wires and"), F("start again from home."), F("RED 12 wires, BLUE 13."), K_BACK | K_NEXT); break;
    case 28: textScreen(F("ROUND 2"), IC_PEOPLE, F("2/2"), F("Now find a way for both"), F("to reach all destinations"), F("together."), K_BACK | K_NEXT); break;
    case 29: textScreen(F("HINT"), IC_TARGET, NULL, F("How could you share the"), F("tasks so all destinations"), F("fit each wire budget?"), K_BACK | K_NEXT); break;
    case 30: timerScreen(F("ROUND 2"), F("5 minutes")); break;
    case 31: textScreen(F("DISCUSS"), IC_SPEECH, F("1/2"), F("The streets stay the same."), F("Only the division of care"), F("has changed."), K_BACK | K_NEXT); break;
    case 32: textScreen(F("DISCUSS"), IC_SPEECH, F("2/2"), F("So who carries the chain"), F("in your household?"), NULL, K_BACK | K_NEXT); break;
    case 33: textScreen(F("THANK YOU"), IC_FLAG, NULL, F("Thank you for playing!"), F("Please put all wires back"), F("in the holders."), K_START | K_NEW); break;
  }
}

void goTo(byte n) {
  screen = n;
  showScreen(n);
}

// ---------- buttons ----------

const byte EV_NONE  = 0;
const byte EV_NEXT  = 1;
const byte EV_BACK  = 2;
const byte EV_RESET = 3;

const byte BTN_PINS[2] = {PIN_NEXT, PIN_BACK};
byte btnLow[2]  = {0, 0};
byte btnHigh[2] = {0, 0};
bool btnDown[2] = {false, false};
bool comboUsed  = false;

// Call every 20 ms. A state counts after 3 equal reads in a row (60 ms).
byte pollButtons() {
  byte ev = EV_NONE;
  for (byte i = 0; i < 2; i++) {
    if (digitalRead(BTN_PINS[i]) == LOW) {
      btnHigh[i] = 0;
      if (!btnDown[i] && ++btnLow[i] >= 3) {
        btnDown[i] = true;
        btnLow[i] = 0;
      }
    } else {
      btnLow[i] = 0;
      if (btnDown[i] && ++btnHigh[i] >= 3) {
        btnDown[i] = false;
        btnHigh[i] = 0;
        if (!comboUsed) ev = (i == 0) ? EV_NEXT : EV_BACK;   // act on release
      }
    }
  }
  if (btnDown[0] && btnDown[1] && !comboUsed) {     // both down: RESET once
    comboUsed = true;
    ev = EV_RESET;
  }
  if (!btnDown[0] && !btnDown[1]) comboUsed = false;
  return ev;
}

byte runTimer() {
  uint16_t tenths = ROUND_SECONDS * 10;
  int shown = -1;
  while (tenths > 0) {
    uint16_t secs = (tenths + 9) / 10;
    bool show = secs > 10 || (tenths / 5) % 2 == 0;   // flash in the last 10 s
    int state = secs * 2 + show;
    if (state != shown) {
      drawTime(secs, show);
      shown = state;
    }
    for (byte k = 0; k < 5; k++) {         // 5 x 20 ms = 0.1 s
      display.delay(20);
      byte ev = pollButtons();
      if (ev == EV_RESET) {                // both buttons: restart the timer
        tenths = ROUND_SECONDS * 10 + 1;
        shown = -1;
        break;
      }
      if (ev) return ev;
    }
    tenths--;
  }
  return EV_NEXT;
}

// ---------- main ----------

void setup() {
  pinMode(PIN_NEXT, INPUT_PULLUP);
  pinMode(PIN_BACK, INPUT_PULLUP);
  display.begin();
  display.setTextWrap(false);
  goTo(0);
}

void loop() {
  byte ev = EV_NONE;
  if (keys & K_TIMER) {
    ev = runTimer();                       // round screen: 5 minute countdown
  } else {
    while (ev == EV_NONE) {                // text screen: wait for a button
      display.delay(20);
      ev = pollButtons();
    }
  }
  if (ev == EV_RESET)     goTo(0);
  else if (ev == EV_BACK) goTo(screen > 0 ? screen - 1 : 0);
  else                    goTo(screen + 1 >= SCREEN_COUNT ? 0 : screen + 1);
}
