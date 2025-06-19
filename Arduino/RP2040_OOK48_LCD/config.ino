

uint8_t ch;
TFT_eSPI_Button cfgKbd[8];

void configMain(void){
/*
Configuration items

1S/2S Timing
6/8/10 character QTH locator
Clear 'EEPROM' - allows new GPS and clears all messages

*/


bool done = false;
bool cfgLoop = false;
  while(!cfgLoop){
  drawCFGKbd();
  delay(50); // UI debouncing
    while(!done)
    {

        // Pressed will be set true is there is a valid touch on the screen
        bool pressed = tft.getTouch(&t_x, &t_y);
        // / Check if any key coordinate boxes contain the touch coordinates
        for (uint8_t b = 0; b < 8; b++) 
        {
          if (pressed && cfgKbd[b].contains(t_x, t_y)) 
          {
            cfgKbd[b].press(true);  // tell the button it is pressed
          }
          else 
          {
            cfgKbd[b].press(false);  // tell the button it is NOT pressed
          }
        }

        // Check if any key has changed state
        for (uint8_t b = 0; b < 8; b++) 
        {
          if (cfgKbd[b].justPressed()) 
          {
            ch=b;
            done = 1;
          }
        }
    }

  if (ch==7) cfgLoop = true;
  done = false;
  if (ch < 3) configQth(ch);
  if (ch == 5) configClr(ch);
  if (ch > 2) configTiming(ch); 
  // to-do EEPROM clr

  }
}

void configTiming(uint8_t x){
  switch (x){
    case 3:
      halfRate = false;
      break;
    case 4:
      halfRate = true;
      break;
    default:
      break;
  }
}
void configQth(uint8_t x){
  switch (x){
    case 0:
      locatorLength = 6;
      break;
    case 1:
      locatorLength = 8;
      break;
    case 2:
      locatorLength = 10;
      break;    
    default:
      break;
    }
}

void configClr(uint8_t x){


}
void drawCFGKbd(void){
char congfglabels[8][5]={"6", "8", "10", "1s", "2s", "N", "CONF","EXIT"};

uint16_t cfgTextcolour;

  // Draw pad background
  tft.fillRect(CFG_X, 0, CFG_WIDTH, CFG_HEIGHT, TFT_DARKGREY);


  // Line 1
  tft.setFreeFont(&FreeSans12pt7b);  // Font
  // Draw the string, the value returned is the width in pixels
  tft.setTextColor(TFT_CYAN);
  tft.drawString("Set Locator length", CFG_TEXTLEFT, CFG_LINESPACING*2);
  
  tft.drawString("Period ", CFG_TEXTLEFT, CFG_LINESPACING*4);
  tft.setTextColor(TFT_RED);
  tft.drawString("Wipe EEPROM", CFG_TEXTLEFT, CFG_LINESPACING*6);
  tft.setFreeFont(KB_FONT); 


      if (locatorLength == 6) cfgTextcolour = TFT_GREEN; else cfgTextcolour = TFT_WHITE;
      cfgKbd[0].initButton(&tft, CFG_BUTTONSLEFT + CFG_W/2,
                        CFG_LINESPACING*2 + CFG_LINESPACING/2, // x, y, w, h, outline, fill, text
                        CFG_W, CFG_H, TFT_WHITE, TFT_BLUE, cfgTextcolour,
                        congfglabels[0], CFG_TEXTSIZE);
      cfgKbd[0].drawButton(); 
      if (locatorLength == 8) cfgTextcolour = TFT_GREEN; else cfgTextcolour = TFT_WHITE;
      cfgKbd[1].initButton(&tft, CFG_BUTTONSLEFT + CFG_W + CFG_W/2 + CFG_SPACING_X,
                        CFG_LINESPACING*2 + CFG_LINESPACING/2, // x, y, w, h, outline, fill, text
                        CFG_W, CFG_H, TFT_WHITE, TFT_BLUE, cfgTextcolour,
                        congfglabels[1], CFG_TEXTSIZE);
      cfgKbd[1].drawButton(); 
      if (locatorLength == 10) cfgTextcolour = TFT_GREEN; else cfgTextcolour = TFT_WHITE;
      cfgKbd[2].initButton(&tft, CFG_BUTTONSLEFT + CFG_W*2 + CFG_W/2 + 2*CFG_SPACING_X,
                        CFG_LINESPACING*2 + CFG_LINESPACING/2, // x, y, w, h, outline, fill, text
                        CFG_W, CFG_H, TFT_WHITE, TFT_BLUE, cfgTextcolour,
                        congfglabels[2], CFG_TEXTSIZE);
      cfgKbd[2].drawButton(); 

      if (!halfRate) cfgTextcolour = TFT_GREEN; else cfgTextcolour = TFT_WHITE;
      cfgKbd[3].initButton(&tft, CFG_BUTTONSLEFT + CFG_W/2,
                        CFG_LINESPACING*4 + CFG_LINESPACING/2, // x, y, w, h, outline, fill, text
                        CFG_W, CFG_H, TFT_WHITE, TFT_BLUE, cfgTextcolour,
                        congfglabels[3], CFG_TEXTSIZE);
      cfgKbd[3].drawButton(); 
      if (halfRate) cfgTextcolour = TFT_GREEN; else cfgTextcolour = TFT_WHITE;
      cfgKbd[4].initButton(&tft, CFG_BUTTONSLEFT + CFG_W + CFG_W/2 + CFG_SPACING_X,
                        CFG_LINESPACING*4 + CFG_LINESPACING/2, // x, y, w, h, outline, fill, text
                        CFG_W, CFG_H, TFT_WHITE, TFT_BLUE, cfgTextcolour,
                        congfglabels[4], CFG_TEXTSIZE);
      cfgKbd[4].drawButton(); 
      cfgKbd[5].initButton(&tft, CFG_BUTTONSLEFT + CFG_W + CFG_W/2 + CFG_SPACING_X,
                        CFG_LINESPACING*6 + CFG_LINESPACING/2, // x, y, w, h, outline, fill, text
                        CFG_W, CFG_H, TFT_WHITE, TFT_RED, TFT_WHITE,
                        congfglabels[5], CFG_TEXTSIZE);
      cfgKbd[5].drawButton(); 
      cfgKbd[7].initButton(&tft, CFG_WIDTH - (CFG_W),
                        CFG_LINESPACING*14 + CFG_LINESPACING/2, // x, y, w, h, outline, fill, text
                        CFG_W, CFG_H, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                        congfglabels[7],  CFG_TEXTSIZE);
      cfgKbd[7].drawButton(); 
}
