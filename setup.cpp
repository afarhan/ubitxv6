#include <Arduino.h>
#include <EEPROM.h>
#include "morse.h"
#include "settings.h"
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

void displayDialog(const __FlashStringHelper* title, const __FlashStringHelper* instructions){
  strcpy_P(b,(const char*)title);
  strcpy_P(c,(const char*)instructions);
  displayClear(COLOR_BACKGROUND);
  displayRect(LAYOUT_OUTER_BORDER_X,LAYOUT_OUTER_BORDER_Y,LAYOUT_OUTER_BORDER_WIDTH,LAYOUT_OUTER_BORDER_HEIGHT, COLOR_ACTIVE_BORDER);
  displayRect(LAYOUT_INNER_BORDER_X,LAYOUT_INNER_BORDER_Y,LAYOUT_INNER_BORDER_WIDTH,LAYOUT_INNER_BORDER_HEIGHT, COLOR_ACTIVE_BORDER);
  displayText(b, LAYOUT_TITLE_X, LAYOUT_TITLE_Y, LAYOUT_TITLE_WIDTH, LAYOUT_TITLE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_ACTIVE_BORDER);
  displayText(c, LAYOUT_INSTRUCTION_TEXT_X, LAYOUT_INSTRUCTION_TEXT_Y, LAYOUT_INSTRUCTION_TEXT_WIDTH, LAYOUT_INSTRUCTION_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
}

void printCarrierFreq(unsigned long freq)
{
  formatFreq(freq,c,sizeof(c));
  displayText(c, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_BACKGROUND);
}

void setupFreq(){
  displayDialog(F("Set Frequency"),F("Push TUNE to Save"));

  //round off the the nearest khz
  {
    uint32_t freq = GetActiveVfoFreq();
    freq = (freq/1000l)* 1000l;
    setFrequency(freq);
  }

  strcpy_P(c,(const char*)F("You should have a"));
  displayText(c, LAYOUT_SETTING_VALUE_X, LAYOUT_ITEM_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  strcpy_P(c,(const char*)F("signal exactly at"));
  displayText(c, LAYOUT_SETTING_VALUE_X, LAYOUT_ITEM_Y + 1*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  ltoa(GetActiveVfoFreq()/1000L, c, 10);
  strcat_P(c,(const char*)F(" KHz"));
  displayText(c, LAYOUT_SETTING_VALUE_X, LAYOUT_ITEM_Y + 2*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  strcpy_P(c,(const char*)F("Rotate to zerobeat"));
  displayText(c, LAYOUT_SETTING_VALUE_X, LAYOUT_ITEM_Y + 4*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  
  ltoa(globalSettings.oscillatorCal, b, 10);
  displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_BACKGROUND);
  //keep clear of any previous button press
  while (btnDown())
    active_delay(100);
  active_delay(100);

  while (!btnDown())
  {
    int knob = enc_read();
    if(knob != 0){
      globalSettings.oscillatorCal += knob * 875;
    }
    else{
      continue; //don't update the frequency or the display
    }

    si5351bx_setfreq(0, globalSettings.usbCarrierFreq); //set back the carrier oscillator anyway, cw tx switches it off
    si5351_set_calibration(globalSettings.oscillatorCal);
    setFrequency(GetActiveVfoFreq());
    
    ltoa(globalSettings.oscillatorCal, b, 10);
    displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_BACKGROUND);
  }

  SaveSettingsToEeprom();
  initOscillators();
  si5351_set_calibration(globalSettings.oscillatorCal);
  setFrequency(GetActiveVfoFreq());

  //debounce and delay
  while(btnDown())
    active_delay(50);
  active_delay(100);
}

void setupBFO(){
  displayDialog(F("Set BFO"),F("Press TUNE to Save")); 

  si5351bx_setfreq(0, globalSettings.usbCarrierFreq);
  printCarrierFreq(globalSettings.usbCarrierFreq);

  while (!btnDown()){
    int knob = enc_read();
    if(knob != 0){
      globalSettings.usbCarrierFreq -= 50 * knob;
    }
    else{
      continue; //don't update the frequency or the display
    }
      
    si5351bx_setfreq(0, globalSettings.usbCarrierFreq);
    setFrequency(GetActiveVfoFreq());
    printCarrierFreq(globalSettings.usbCarrierFreq);
    
    active_delay(100);
  }

  SaveSettingsToEeprom();
  si5351bx_setfreq(0, globalSettings.usbCarrierFreq);
  setFrequency(GetActiveVfoFreq());
}

void setupCwDelay(){
  int knob = 0;
  int prev_cw_delay;

  displayDialog(F("Set CW T/R Delay"),F("Press tune to Save")); 

  prev_cw_delay = globalSettings.cwActiveTimeoutMs;

  ltoa(globalSettings.cwActiveTimeoutMs, b, 10);
  strcat_P(b,(const char*)F(" msec"));
  displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);

  while (!btnDown()){
    knob = enc_read();

    if (knob < 0 && globalSettings.cwActiveTimeoutMs > 100)
      globalSettings.cwActiveTimeoutMs -= 100;
    else if (knob > 0 && globalSettings.cwActiveTimeoutMs < 1000)
      globalSettings.cwActiveTimeoutMs += 100;
    else
      continue; //don't update the frequency or the display

    ltoa(globalSettings.cwActiveTimeoutMs, b, 10);
    strcat_P(b,(const char*)F(" msec"));
    displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
      
  }

  SaveSettingsToEeprom();
}

void formatKeyerEnum(char* output, const KeyerMode_e mode)
{
  if(KeyerMode_e::KEYER_STRAIGHT == mode){
    strcpy_P(output,(const char*)F("< Hand Key >"));
  }
  else if(KeyerMode_e::KEYER_IAMBIC_A == mode){
    strcpy_P(output,(const char*)F("< Iambic A >"));
  }
  else{
    strcpy_P(output,(const char*)F("< Iambic B >"));
  }
}

void setupKeyer(){
  displayDialog(F("Set CW Keyer"),F("Press tune to Save")); 

  int knob = 0;
  uint32_t tmp_mode = globalSettings.keyerMode;
  formatKeyerEnum(c, tmp_mode);
  displayText(c, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);

  while (!btnDown())
  {
    knob = enc_read();
    if(knob == 0){
      active_delay(50);
      continue;
    }
    if(knob < 0 && tmp_mode > KeyerMode_e::KEYER_STRAIGHT){
      tmp_mode--;
    }
    if(knob > 0 && tmp_mode < KeyerMode_e::KEYER_IAMBIC_B){
      tmp_mode++;
    }

    formatKeyerEnum(c,tmp_mode);
    displayText(c, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
  }

  active_delay(500);

  globalSettings.keyerMode = tmp_mode;
  SaveSettingsToEeprom();
}

void setupCwSpeed()
{
  displayDialog(F("Set CW Speed (WPM)"),F("Press tune to Save"));

  unsigned int wpm = 1200/globalSettings.cwDitDurationMs;

  itoa(wpm, b, 10);
  displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);

  while (!btnDown()){
    int knob = enc_read();

    if (knob < 0 && wpm > 1)
      --wpm;
    else if (knob > 0 && wpm < 100)
      ++wpm;
    else
      continue;//don't update the frequency or the display

    itoa(wpm, b, 10);
    displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
  }

  globalSettings.cwDitDurationMs = 1200/wpm;
  SaveSettingsToEeprom();
}

void setupCwTone(){
  displayDialog(F("Set CW Tone (Hz)"),F("Press tune to Save"));

  tone(CW_TONE, globalSettings.cwSideToneFreq);
  itoa(globalSettings.cwSideToneFreq, b, 10);
  displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);

  while(!btnDown()){
    int knob = enc_read();

    if (knob > 0 && globalSettings.cwSideToneFreq < 2000)
      globalSettings.cwSideToneFreq += 10;
    else if (knob < 0 && globalSettings.cwSideToneFreq > 100 )
      globalSettings.cwSideToneFreq -= 10;
    else
      continue; //don't update the frequency or the display
        
    tone(CW_TONE, globalSettings.cwSideToneFreq);
    itoa(globalSettings.cwSideToneFreq, b, 10);
    displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
  }
  noTone(CW_TONE);

  SaveSettingsToEeprom();
}

void setupResetAll()
{
  displayDialog(F("Reset all cals and settings?"),F("Press tune to Confirm"));
  strcpy_P(b,(const char*)F("No"));
  displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);

  bool reset_all = false;
  while(!btnDown()){
    int knob = enc_read();
    
    if(knob > 0){
      reset_all = true;
    }
    else if(knob < 0){
      reset_all = false;
    }
    else{
      continue;
    }

    if(reset_all){
      strcpy_P(b,(const char*)F("Yes"));
    }
    else{
      strcpy_P(b,(const char*)F("No"));
    }
    
    displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND);
  }

  while(btnDown()){
    active_delay(50);
  }
  active_delay(50);

  if(reset_all){
    LoadDefaultSettings();
    SaveSettingsToEeprom();
    setup();
  }
}

struct MenuItem_t {
  const char* const ItemName;
  const void (*OnSelect)();
};

void runMenu(const MenuItem_t* const menu_items, const uint16_t num_items);
#define RUN_MENU(menu) runMenu(menu,sizeof(menu)/sizeof(menu[0]))

const char MT_CAL [] PROGMEM = "Calibrations";
const char MI_SET_FREQ [] PROGMEM = "Local Oscillator";
const char MI_SET_BFO [] PROGMEM = "Beat Frequency Osc (BFO)";
const char MI_TOUCH [] PROGMEM = "Touch Screen";
const MenuItem_t calibrationMenu [] PROGMEM {
  {MT_CAL,nullptr},//Title
  {MI_SET_FREQ,setupFreq},
  {MI_SET_BFO,setupBFO},
  {MI_TOUCH,setupTouch},
};
void runCalibrationMenu(){RUN_MENU(calibrationMenu);}

const char MT_CW [] PROGMEM = "CW/Morse Setup";
const char MI_CW_SPEED [] PROGMEM = "Play Speed (WPM)";
const char MI_CW_TONE [] PROGMEM = "Tone Frequency";
const char MI_CW_DELAY [] PROGMEM = "Tx/Rx Switching Delay";
const char MI_CW_KEYER [] PROGMEM = "Keyer Type";
const MenuItem_t cwMenu [] PROGMEM {
  {MT_CW,nullptr},//Title
  {MI_CW_SPEED,setupCwSpeed},
  {MI_CW_TONE,setupCwTone},
  {MI_CW_DELAY,setupCwDelay},
  {MI_CW_KEYER,setupKeyer},
};
void runCwMenu(){RUN_MENU(cwMenu);}

const char MT_SETTINGS [] PROGMEM = "Settings";
const char MI_RESET [] PROGMEM = "Reset all Cals/Settings";
const MenuItem_t mainMenu [] PROGMEM {
  {MT_SETTINGS,nullptr},//Title
  {MT_CAL,runCalibrationMenu},
  {MT_CW,runCwMenu},
  {MI_RESET,setupResetAll},
};

const char MI_EXIT [] PROGMEM = "Exit";
const MenuItem_t exitMenu PROGMEM = {MI_EXIT,nullptr};

void drawMenu(const MenuItem_t* const items, const uint16_t num_items)
{
  displayClear(COLOR_BACKGROUND);
  MenuItem_t mi = {"",nullptr};
  memcpy_P(&mi,&items[0],sizeof(mi));
  strcpy_P(b,mi.ItemName);
  displayText(b, LAYOUT_TITLE_X, LAYOUT_TITLE_Y, LAYOUT_TITLE_WIDTH, LAYOUT_TITLE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_ACTIVE_BORDER);
  for(unsigned int i = 1; i < num_items; ++i){
    memcpy_P(&mi,&items[i],sizeof(mi));
    strcpy_P(b,mi.ItemName);
    displayText(b, LAYOUT_ITEM_X, LAYOUT_ITEM_Y + (i-1)*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_INACTIVE_BORDER);
  }
  memcpy_P(&mi,&exitMenu,sizeof(mi));
  strcpy_P(b,mi.ItemName);
  displayText(b, LAYOUT_ITEM_X, LAYOUT_ITEM_Y + (num_items-1)*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_INACTIVE_BORDER);
}

void movePuck(unsigned int old_index, unsigned int new_index)
{
  //Don't update if we're already on the right selection
  if(old_index == new_index){
    return;
  }
  else if(((unsigned int)-1) != old_index){
    //Clear old
    displayRect(LAYOUT_ITEM_X, LAYOUT_ITEM_Y + (old_index*LAYOUT_ITEM_PITCH_Y), LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_INACTIVE_BORDER);
  }
  //Draw new
  displayRect(LAYOUT_ITEM_X, LAYOUT_ITEM_Y + (new_index*LAYOUT_ITEM_PITCH_Y), LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_ACTIVE_BORDER);
}

void runMenu(const MenuItem_t* const menu_items, const uint16_t num_items)
{
  static const unsigned int COUNTS_PER_ITEM = 10;
  const unsigned int MAX_KNOB_VALUE = num_items*COUNTS_PER_ITEM - 1;
  int knob_sum = 0;
  unsigned int old_index = 0;

  drawMenu(menu_items,num_items);
  movePuck(1,0);//Force draw of puck

   //wait for the button to be raised up
  while(btnDown()){
    active_delay(50);
  }
  active_delay(50);  //debounce
 
  while (true){
    knob_sum += enc_read();
    if(knob_sum < 0){
      knob_sum = 0;
    }
    else if(MAX_KNOB_VALUE < knob_sum){
      knob_sum = MAX_KNOB_VALUE;
    }

    uint16_t index = knob_sum/COUNTS_PER_ITEM;
    movePuck(old_index,index);
    old_index = index;

    if (!btnDown()){
      active_delay(50);
      continue;
    }

    //wait for the touch to lift off and debounce
    while(btnDown()){
      active_delay(50);
    }
    active_delay(50);//debounce
    
    if(num_items-1 > index){
      MenuItem_t mi = {"",nullptr};
      memcpy_P(&mi,&menu_items[index+1],sizeof(mi));//The 0th element in the array is the title, so offset by 1
      mi.OnSelect();
      drawMenu(menu_items,num_items);//Need to re-render, since whatever ran just now is assumed to have drawn something
      old_index = -1;//Force redraw
    }
    else{
      break;
    }
  }

  //debounce the button
  while(btnDown()){
    active_delay(50);
  }
  active_delay(50);//debounce
}

void doSetup2(){
  RUN_MENU(mainMenu);
  guiUpdate();
}
