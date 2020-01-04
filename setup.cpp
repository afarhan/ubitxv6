#include <Arduino.h>
#include <EEPROM.h>
#include "morse.h"
#include "ubitx.h"
#include "nano_gui.h"

/** Menus
 *  The Radio menus are accessed by tapping on the function button. 
 *  - The main loop() constantly looks for a button press and calls doMenu() when it detects
 *  a function button press. 
 *  - As the encoder is rotated, at every 10th pulse, the next or the previous menu
 *  item is displayed. Each menu item is controlled by it's own function.
 *  - Eache menu function may be called to display itself
 *  - Each of these menu routines is called with a button parameter. 
 *  - The btn flag denotes if the menu itme was clicked on or not.
 *  - If the menu item is clicked on, then it is selected,
 *  - If the menu item is NOT clicked on, then the menu's prompt is to be displayed
 */

void setupExit(){
  menuOn = 0;
}

 //this is used by the si5351 routines in the ubitx_5351 file
extern int32_t calibration;
extern uint32_t si5351bx_vcoa;

static const unsigned int COLOR_TEXT = DISPLAY_WHITE;
static const unsigned int COLOR_BACKGROUND = DISPLAY_BLACK;
static const unsigned int COLOR_TITLE_BACKGROUND = DISPLAY_NAVY;
static const unsigned int COLOR_SETTING_BACKGROUND = DISPLAY_NAVY;
static const unsigned int COLOR_ACTIVE_BORDER = DISPLAY_WHITE;
static const unsigned int COLOR_INACTIVE_BORDER = COLOR_BACKGROUND;

static const unsigned int LAYOUT_OUTER_BORDER_X = 10;
static const unsigned int LAYOUT_OUTER_BORDER_Y = 10;
static const unsigned int LAYOUT_OUTER_BORDER_WIDTH = 300;
static const unsigned int LAYOUT_OUTER_BORDER_HEIGHT = 220;

static const unsigned int LAYOUT_INNER_BORDER_X = 12;
static const unsigned int LAYOUT_INNER_BORDER_Y = 12;
static const unsigned int LAYOUT_INNER_BORDER_WIDTH = 296;
static const unsigned int LAYOUT_INNER_BORDER_HEIGHT = 216;

static const unsigned int LAYOUT_TITLE_X = LAYOUT_INNER_BORDER_X;
static const unsigned int LAYOUT_TITLE_Y = LAYOUT_INNER_BORDER_Y;
static const unsigned int LAYOUT_TITLE_WIDTH = LAYOUT_INNER_BORDER_WIDTH;
static const unsigned int LAYOUT_TITLE_HEIGHT = 35;

static const unsigned int LAYOUT_ITEM_X = 30;
static const unsigned int LAYOUT_ITEM_Y = LAYOUT_TITLE_Y + LAYOUT_TITLE_HEIGHT + 5;
static const unsigned int LAYOUT_ITEM_WIDTH = 260;
static const unsigned int LAYOUT_ITEM_HEIGHT = 30;
static const unsigned int LAYOUT_ITEM_PITCH_Y = LAYOUT_ITEM_HEIGHT + 1;

static const unsigned int LAYOUT_SETTING_VALUE_X = LAYOUT_ITEM_X;
static const unsigned int LAYOUT_SETTING_VALUE_Y = LAYOUT_ITEM_Y + 3*LAYOUT_ITEM_PITCH_Y;
static const unsigned int LAYOUT_SETTING_VALUE_WIDTH = LAYOUT_ITEM_WIDTH;
static const unsigned int LAYOUT_SETTING_VALUE_HEIGHT = LAYOUT_ITEM_HEIGHT;

static const unsigned int LAYOUT_INSTRUCTION_TEXT_X = 20;
static const unsigned int LAYOUT_INSTRUCTION_TEXT_Y = LAYOUT_ITEM_Y + 5*LAYOUT_ITEM_PITCH_Y;
static const unsigned int LAYOUT_INSTRUCTION_TEXT_WIDTH = LAYOUT_ITEM_WIDTH;
static const unsigned int LAYOUT_INSTRUCTION_TEXT_HEIGHT = LAYOUT_ITEM_HEIGHT;

void displayDialog(char *title, char *instructions){
  displayClear(COLOR_BACKGROUND);
  displayRect(LAYOUT_OUTER_BORDER_X,LAYOUT_OUTER_BORDER_Y,LAYOUT_OUTER_BORDER_WIDTH,LAYOUT_OUTER_BORDER_HEIGHT, COLOR_ACTIVE_BORDER);
  displayRect(LAYOUT_INNER_BORDER_X,LAYOUT_INNER_BORDER_Y,LAYOUT_INNER_BORDER_WIDTH,LAYOUT_INNER_BORDER_HEIGHT, COLOR_ACTIVE_BORDER);
  displayText(title, LAYOUT_TITLE_X, LAYOUT_TITLE_Y, LAYOUT_TITLE_WIDTH, LAYOUT_TITLE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_ACTIVE_BORDER);
  displayText(instructions, LAYOUT_INSTRUCTION_TEXT_X, LAYOUT_INSTRUCTION_TEXT_Y, LAYOUT_INSTRUCTION_TEXT_WIDTH, LAYOUT_INSTRUCTION_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
}

void printCarrierFreq(unsigned long freq){

  memset(c, 0, sizeof(c));
  memset(b, 0, sizeof(b));

  ultoa(freq, b, DEC);
  
  strncat(c, b, 2);
  strcat(c, ".");
  strncat(c, &b[2], 3);
  strcat(c, ".");
  strncat(c, &b[5], 1);
  displayText(c, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_BACKGROUND);
}

void setupFreq(){
  int knob = 0;
  int32_t prev_calibration;

  displayDialog("Set Frequency", "Push TUNE to Save"); 

  //round off the the nearest khz
  frequency = (frequency/1000l)* 1000l;
  setFrequency(frequency);
  
  displayText("You should have a", LAYOUT_SETTING_VALUE_X, LAYOUT_ITEM_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  displayText("signal exactly at ", LAYOUT_SETTING_VALUE_X, LAYOUT_ITEM_Y + 1*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  ltoa(frequency/1000l, c, 10);
  strcat(c, " KHz");
  displayText(c, LAYOUT_SETTING_VALUE_X, LAYOUT_ITEM_Y + 2*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  displayText("Rotate to zerobeat", LAYOUT_SETTING_VALUE_X, LAYOUT_ITEM_Y + 4*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  
  ltoa(calibration, b, 10);
  displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_BACKGROUND);
  //keep clear of any previous button press
  while (btnDown())
    active_delay(100);
  active_delay(100);
   
  prev_calibration = calibration;
  calibration = 0;

//  ltoa(calibration/8750, c, 10);
//  strcpy(b, c);
//  strcat(b, "Hz");
//  printLine2(b);     

  while (!btnDown())
  {
   knob = enc_read();
   if (knob != 0)
      calibration += knob * 875;
 /*   else if (knob < 0)
      calibration -= 875; */
    else  
      continue; //don't update the frequency or the display
 
    si5351bx_setfreq(0, usbCarrier);  //set back the cardrier oscillator anyway, cw tx switches it off  
    si5351_set_calibration(calibration);
    setFrequency(frequency);

    //displayRawText("Rotate to zerobeat", 20, 120, DISPLAY_CYAN, DISPLAY_NAVY);
    
    ltoa(calibration, b, 10);
    displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_BACKGROUND);
  }

  EEPROM.put(MASTER_CAL, calibration);
  initOscillators();
  si5351_set_calibration(calibration);
  setFrequency(frequency);    

  //debounce and delay
  while(btnDown())
    active_delay(50);
  active_delay(100);
}

void setupBFO(){
  int knob = 0;
  unsigned long prevCarrier;
   
  prevCarrier = usbCarrier;

  displayDialog("Set BFO", "Press TUNE to Save"); 
  
  usbCarrier = 11053000l;
  si5351bx_setfreq(0, usbCarrier);
  printCarrierFreq(usbCarrier);

  while (!btnDown()){
    knob = enc_read();

    if (knob != 0)
      usbCarrier -= 50 * knob;
    else
      continue; //don't update the frequency or the display
      
    si5351bx_setfreq(0, usbCarrier);
    setFrequency(frequency);
    printCarrierFreq(usbCarrier);
    
    active_delay(100);
  }

  EEPROM.put(USB_CAL, usbCarrier);  
  si5351bx_setfreq(0, usbCarrier);          
  setFrequency(frequency);
  updateDisplay();
  menuOn = 0; 
}

void setupCwDelay(){
  int knob = 0;
  int prev_cw_delay;

  displayDialog("Set CW T/R Delay", "Press tune to Save"); 

  active_delay(500);
  prev_cw_delay = cwDelayTime;

  itoa(10 * (int)cwDelayTime, b, 10);
  strcat(b, " msec");
  displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);

  while (!btnDown()){
    knob = enc_read();

    if (knob < 0 && cwDelayTime > 10)
      cwDelayTime -= 10;
    else if (knob > 0 && cwDelayTime < 100)
      cwDelayTime += 10;
    else
      continue; //don't update the frequency or the display

    itoa(10 * (int)cwDelayTime, b, 10);
    strcat(b, " msec");
    displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
      
  }

  EEPROM.put(CW_DELAYTIME, cwDelayTime);

  
//  cwDelayTime = getValueByKnob(10, 1000, 50,  cwDelayTime, "CW Delay>", " msec");

  active_delay(500);
  menuOn = 0;
}

void setupKeyer(){
  int tmp_key, knob;
  
  displayDialog("Set CW Keyer", "Press tune to Save"); 
 
  if (!Iambic_Key)
    displayText("< Hand Key >", LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
  else if (keyerControl & IAMBICB)
    displayText("< Iambic A >", LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
  else 
    displayText("< Iambic B >", LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);

  if (!Iambic_Key)
    tmp_key = 0; //hand key
  else if (keyerControl & IAMBICB)
    tmp_key = 2; //Iambic B
  else 
    tmp_key = 1;
 
  while (!btnDown())
  {
    knob = enc_read();
    if (knob == 0){
      active_delay(50);
      continue;
    }
    if (knob < 0 && tmp_key > 0)
      tmp_key--;
    if (knob > 0)
      tmp_key++;
    if (tmp_key > 2)
      tmp_key = 0;

    if (tmp_key == 0)
      displayText("< Hand Key >", LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
    else if (tmp_key == 1)
      displayText("< Iambic A >", LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
    else if (tmp_key == 2)
      displayText("< Iambic B >", LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
  }

  active_delay(500);
  if (tmp_key == 0)
    Iambic_Key = false;
  else if (tmp_key == 1){
    Iambic_Key = true;
    keyerControl &= ~IAMBICB;
  }
  else if (tmp_key == 2){
    Iambic_Key = true;
    keyerControl |= IAMBICB;
  }
  
  EEPROM.put(CW_KEY_TYPE, tmp_key);
  
  menuOn = 0;
}

const char MI_SET_FREQ [] PROGMEM = "Set Freq...";
const char MI_SET_BFO [] PROGMEM = "Set BFO...";
const char MI_CW_DELAY [] PROGMEM = "CW Delay...";
const char MI_CW_KEYER [] PROGMEM = "CW Keyer...";
const char MI_TOUCH [] PROGMEM = "Touch Screen...";
const char MI_EXIT [] PROGMEM = "Exit";

enum MenuIds {
  MENU_SET_FREQ,
  MENU_SET_BFO,
  MENU_CW_DELAY,
  MENU_CW_KEYER,
  MENU_TOUCH,
  MENU_EXIT,
  MENU_TOTAL
};

const char* const menuItems [MENU_TOTAL] PROGMEM {
  MI_SET_FREQ,
  MI_SET_BFO,
  MI_CW_DELAY,
  MI_CW_KEYER,
  MI_TOUCH,
  MI_EXIT
};

void drawSetupMenu(){
  displayClear(COLOR_BACKGROUND);
  strcpy_P(b,(const char*)F("Setup"));
  displayText(b, LAYOUT_TITLE_X, LAYOUT_TITLE_Y, LAYOUT_TITLE_WIDTH, LAYOUT_TITLE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_ACTIVE_BORDER);
  for(unsigned int i = 0; i < MENU_TOTAL; ++i){
    strcpy_P(b,(const char*)pgm_read_word(&(menuItems[i])));
    displayText(b, LAYOUT_ITEM_X, LAYOUT_ITEM_Y + i*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_INACTIVE_BORDER);
  }
}

void movePuck(int i){
  static int prevPuck = 1;//Start value at 1 so that on init, when we get called with 0, we'll update

  //Don't update if we're already on the right selection
  if(prevPuck == i){
    return;
  }

  //Clear old
  displayRect(LAYOUT_ITEM_X, LAYOUT_ITEM_Y + (prevPuck*LAYOUT_ITEM_PITCH_Y), LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_INACTIVE_BORDER);
  //Draw new
  displayRect(LAYOUT_ITEM_X, LAYOUT_ITEM_Y + (i*LAYOUT_ITEM_PITCH_Y), LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_ACTIVE_BORDER);
  prevPuck = i;
  
}

void doSetup2(){
  static const unsigned int COUNTS_PER_ITEM = 10;
  int select=0, i, btnState;

  drawSetupMenu();
  movePuck(select);

   //wait for the button to be raised up
  while(btnDown())
    active_delay(50);
  active_delay(50);  //debounce
  
  menuOn = 2;
 
  while (menuOn){
    i = enc_read();

    if (i > 0){
      if (select + i < MENU_TOTAL*COUNTS_PER_ITEM)
        select += i;
      movePuck(select/COUNTS_PER_ITEM);
    }
    if (i < 0 && select + i >= 0){
      select += i;      //caught ya, i is already -ve here, so you add it
      movePuck(select/COUNTS_PER_ITEM);
    }

    if (!btnDown()){
      active_delay(50);
      continue;
    }

    //wait for the touch to lift off and debounce
    while(btnDown()){
      active_delay(50);
    }
    active_delay(300);
    
    switch(select/COUNTS_PER_ITEM){
      case MENU_SET_FREQ:
      {
        setupFreq();
        break;
      }
      case MENU_SET_BFO:
      {
        setupBFO();
        break;
      }
      case MENU_CW_DELAY:
      {
        setupCwDelay();
        break;
      }
      case MENU_CW_KEYER:
      {
        setupKeyer();
        break;
      }
      case MENU_TOUCH:
      {
        setupTouch();
        break;
      }
      case MENU_EXIT:
      default:
      {
        menuOn = 0;
        break;
      }
    }//switch
    //redraw
    drawSetupMenu();
  }

  //debounce the button
  while(btnDown())
    active_delay(50);
  active_delay(50);

  checkCAT();
  guiUpdate();
}
