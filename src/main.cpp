/*
 * Electrocard. - An OLED powered business card.
 *
 * Designed an programmed by Michael Teeuw
 * For more info, check: http://michaelteeuw.nl/tagged/electrocard
 *
 */

#include <Arduino.h>
#include <util/delay.h>

#include "TinyOLED.h"

#include "Tetris.h"
#include "System.h"
#include "clickButton.h"
#include <elapsedMillis.h>
#include "Screens.h"

#define STANDBY_TIME 30000
#define STANDBY_TIME_LONG 120000

typedef enum {SCREENS, TETRIS, SYSTEM_INFO} mode_type;
mode_type mode = SCREENS;

Tetris tetris = Tetris();
elapsedMillis sleepTimer;
ClickButton buttons[3] = {
  ClickButton(0, LOW, CLICKBTN_PULLUP),
  ClickButton(1, LOW, CLICKBTN_PULLUP),
  ClickButton(2, LOW, CLICKBTN_PULLUP)
};

void setup()
{
  _delay_ms(100);

  // pinMode(0, INPUT_PULLUP);
  // pinMode(1, INPUT_PULLUP);
  // pinMode(2, INPUT_PULLUP);
  DDRB = 0x00;                //PC as input
  PORTB |= 0b00000111;

  TinyOLED.init();
	TinyOLED.setfont(bitmap_font_small_6bit);

  for (byte i = 0; i < 3; i++) {
    buttons[i].debounceTime   = 20;   // Debounce timer in ms
    buttons[i].multiclickTime = 1;  // Time limit for multi clicks
    buttons[i].longClickTime  = 500; // time until "held-down clicks" register
  }

  // Begin with start image
  Screens.showScreen(2);
}

void sleep() {
  System.sleep();
  sleepTimer = 0;
  mode = SCREENS;
  Screens.showScreen(2);
}

void loop()
{
  // Update all buttons
  for (byte i = 0; i < 3; i++) {
    buttons[i].Update();

    if (buttons[i].clicks > 0 && (mode != TETRIS || tetris.gameOver)) {
      mode = SCREENS;
      Screens.showScreen(i);
    }

    if (mode == TETRIS) {
      if (buttons[i].clicks > 0) tetris.buttonPressed(i);
      if (buttons[i].clicks < 0) tetris.buttonHold(i);
      if (buttons[i].released) tetris.buttonRelease(i);
    }
  }

  if (buttons[0].clicks < 0 && (mode != TETRIS || tetris.gameOver)) {
    tetris.start();
    mode = TETRIS;
  }

  if (buttons[1].clicks < 0 && (mode != TETRIS || tetris.gameOver)) {
    TinyOLED.clear();
    mode = SYSTEM_INFO;
  }

  if (buttons[2].clicks < 0) {
    //Pretend we go off by clearing the display.
    TinyOLED.clear();

    // Wait till the button is released.
    while (!(PINB & (1 << 2))) {
      _delay_ms(100);
    }

    //Good to go! Go to sleep.
    sleep();
  }

  switch (mode) {
    case TETRIS:
      if (!tetris.gameOver) {
        tetris.update();
      } else {
        TinyOLED.setpos(0, 0);
        char scoreText[30];
        sprintf(scoreText, "GAME OVER!\nScore: %d", tetris.playerScore);
        TinyOLED.output_string(scoreText);
      }
    break;
    case SYSTEM_INFO:
      Screens.showScreen(3);
    break;
    default:

    break;
  }

  // Sleep timer reset
  if (buttons[0].clicks != 0 || buttons[1].clicks != 0 || buttons[2].clicks != 0) {
    sleepTimer = 0;
  }
  // Sleep timer initiate
  if (sleepTimer >= ((mode != TETRIS) ? STANDBY_TIME : STANDBY_TIME_LONG)) {
    sleep();
  }

}
