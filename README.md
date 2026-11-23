# Whose 15 Minutes? On-screen instructions

Arduino code for the game screen of *Whose 15 Minutes?*, a board game about care work and the 15-minute city. The game was built by Team 1 (Gendered Mobility) in the seminar "Critical Making: Uncovering Biases, Rapid Prototyping, and Designing for Engagement" (ED130100) at the Technical University of Munich, winter semester 2026/27.

The sketch runs on an Arduino UNO and drives a VGA monitor directly, without a computer. It shows 34 black-and-white instruction screens at 160 x 64 pixels, counts down two rounds of five minutes, and lets players move through the screens with two buttons.

## Repository structure

```
whose-15-minutes-display/
├── README.md
├── .gitignore
└── Whose15Minutes_Display/
    ├── Whose15Minutes_Display.ino   main sketch: screen texts, icons, buttons, round timer
    ├── VGA.h                        VGA library header (160 x 64 pixels)
    └── VGA.cpp                      VGA library: video signal, drawing, delay
```

The Arduino IDE needs the sketch folder to have the same name as the `.ino` file, so keep the folder name `Whose15Minutes_Display`.

## Hardware

- Arduino UNO (ATmega328P). The VGA code uses the AVR timers and inline assembly, so it does not run on boards with other chips, such as the UNO R4.
- VGA monitor and a VGA plug or cable you can wire to
- one 220 ohm resistor
- two push buttons
- jumper wires

## Wiring

| Arduino UNO | Connects to |
|---|---|
| pin 3 | VGA pin 13 (HSYNC) |
| pin 10 | VGA pin 14 (VSYNC) |
| pin 6 | 220 ohm resistor, then VGA pins 1, 2 and 3 joined together (white picture) |
| GND | VGA pins 5, 6, 7, 8 and 10 |
| pin 8 | NEXT button, other side to GND |
| pin 12 | BACK button, other side to GND |

The buttons need no resistors, because the sketch switches on the internal pull-ups. Pin 7 also carries the video signal, so the picture still appears if the resistor is wired to pin 7 instead of pin 6 (shifted by one pixel).

## Buttons

- **NEXT** (pin 8): next screen. On a round screen, it ends the round early.
- **BACK** (pin 12): previous screen.
- **NEXT and BACK together**: back to the first screen. On a round screen, this restarts the five-minute timer instead.

A press counts when the button is released, so pressing both buttons is not read as NEXT or BACK first.

## Installation

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. In the Library Manager, install the **Adafruit GFX Library** and confirm the dependencies it asks for (Adafruit BusIO).
3. Open `Whose15Minutes_Display/Whose15Minutes_Display.ino`.
4. Select the board **Arduino UNO** and the right port, then upload.

## Adapting the game

- **Round length:** `ROUND_SECONDS` in the `.ino` file (300 seconds).
- **Screen texts:** `showScreen()` in the `.ino` file. A text screen has a title in large type and up to three lines of at most 26 characters. If you add or remove screens, update `SCREEN_COUNT`.
- **Timing:** the VGA library switches off Timer0, so `delay()` and `millis()` do not work. Use `display.delay()` instead.
- **More buttons:** do not use pins 0 to 7, because the VGA code writes the whole of PORTD for every pixel and their pull-ups do not stay on. Pins 9, 11 and A0 to A5 are free.
- **Picture position:** `STARTLINE` in `VGA.h` moves the picture up or down on the monitor.

## Screens

| No. | Title | Text |
|---|---|---|
| 1 | 15 MINUTES | Welcome to the 15-minute city! Everything should be a short walk from home. |
| 2 | 15 MINUTES | The idea: work, shops, health and school are all 15 minutes away on foot. |
| 3 | PLAYERS | At least in theory. Two players share the flat in the middle. |
| 4 | PLAYERS | RED is the care persona (player 1). BLUE is the earner persona (player 2). |
| 5 | HOW TO PLAY | The game has two rounds of 5 minutes. Then you talk about the result. |
| 6 | THE CITY | Each street corner has a breadboard. Home is the flat at the centre corner. |
| 7 | THE CITY | Ten buildings have an LED. It turns on when your wires reach it, no gaps. |
| 8 | THE WIRES | One wire bridges one street, from one corner to the next corner. |
| 9 | THE WIRES | Start at home. Lay your wires along the streets from corner to corner. |
| 10 | THE WIRES | Plug each wire end into the breadboard at the corner. No gaps allowed. |
| 11 | THE GOAL | Reach all your destinations within your wire budget and the time. |
| 12 | THE GOAL | Connect only your own destinations. An LED does not know whose wire it is. |
| 13 | ROUND 1 | Each destination on the board is marked RED or BLUE. Find your colour. |
| 14 | ROUND 1 | RED has 12 wires. Destinations: part-time job, daycare, |
| 15 | ROUND 1 | paediatrician, pharmacy, supermarket, post office, parents' flat. |
| 16 | ROUND 1 | BLUE has 13 wires. Destinations: full-time job, gym, hardware store. |
| 17 | WHY 12/13? | RED walks with the child and carries bags, so RED gets one wire less. |
| 18 | ROUND 1 | Both start at home. Each player connects only their own destinations. |
| 19 | ROUND 1 | You do not need to walk back home. Go on from each destination you reach. |
| 20 | ATTENTION | RED goes to work first, then to the daycare and then to the paediatrician. |
| 21 | ROUND 1 | You have 5 minutes. Press NEXT to start the timer. |
| 22 | ROUND 1 | 5 minutes countdown (NEXT ends the round, both buttons restart it) |
| 23 | TIME UP | Stop and leave all wires in place. Count the LEDs that are on. |
| 24 | THE BIAS | The 15-minute city plans single trips from home. Care work is left out. |
| 25 | THE BIAS | A care day links many stops with fixed times. Close stops, long day. |
| 26 | THE BIAS | Women make 58% of all care trips. Care is almost a third of their mobility. |
| 27 | THE BIAS | So the 15-minute city fits men's days better than women's days. |
| 28 | ROUND 2 | Pull out all wires and start again from home. RED 12 wires, BLUE 13. |
| 29 | ROUND 2 | Now find a way for both to reach all destinations together. |
| 30 | HINT | How could you share the tasks so all destinations fit each wire budget? |
| 31 | ROUND 2 | 5 minutes countdown (NEXT ends the round, both buttons restart it) |
| 32 | DISCUSS | The streets stay the same. Only the division of care has changed. |
| 33 | DISCUSS | So who carries the chain in your household? |
| 34 | THANK YOU | Thank you for playing! Please put all wires back in the holders. |

## Credits

`VGA.h` and `VGA.cpp` are a modified version of the Arduino VGA library from [simple-circuit.com](https://simple-circuit.com/), which is based on the [VGAX library](https://github.com/smaffer/vgax). The changes are a picture height of 64 instead of 80 pixels, a picture centred on the monitor (`STARTLINE` 176), and the video output on pin 6 in addition to pin 7. The drawing functions come from the [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library), which is installed separately.

## Team

Arsham Hooshangi, Julius Grüne, Maximilian Michels, Noah Reitinger (Technical University of Munich, TUM)
