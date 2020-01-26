#include <Arduino.h>
#include "morse.h"
#include "nano_gui.h"
#include "setup.h"
#include "settings.h"
#include "ubitx.h"

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

static const unsigned int LAYOUT_INSTRUCTIONS_TEXT_X = 20;
static const unsigned int LAYOUT_INSTRUCTIONS_TEXT_Y = LAYOUT_ITEM_Y;
static const unsigned int LAYOUT_INSTRUCTIONS_TEXT_WIDTH = LAYOUT_ITEM_WIDTH;
static const unsigned int LAYOUT_INSTRUCTIONS_TEXT_HEIGHT = LAYOUT_SETTING_VALUE_Y - LAYOUT_ITEM_Y - 1;

static const unsigned int LAYOUT_CONFIRM_TEXT_X = 20;
static const unsigned int LAYOUT_CONFIRM_TEXT_Y = LAYOUT_ITEM_Y + 5*LAYOUT_ITEM_PITCH_Y;
static const unsigned int LAYOUT_CONFIRM_TEXT_WIDTH = LAYOUT_ITEM_WIDTH;
static const unsigned int LAYOUT_CONFIRM_TEXT_HEIGHT = LAYOUT_ITEM_HEIGHT;

void displayDialog(const char* title,
                   const char* instructions){
  displayClear(COLOR_BACKGROUND);
  displayRect(LAYOUT_OUTER_BORDER_X,LAYOUT_OUTER_BORDER_Y,LAYOUT_OUTER_BORDER_WIDTH,LAYOUT_OUTER_BORDER_HEIGHT, COLOR_ACTIVE_BORDER);
  displayRect(LAYOUT_INNER_BORDER_X,LAYOUT_INNER_BORDER_Y,LAYOUT_INNER_BORDER_WIDTH,LAYOUT_INNER_BORDER_HEIGHT, COLOR_ACTIVE_BORDER);
  strncpy_P(b,title,sizeof(b));
  displayText(b, LAYOUT_TITLE_X, LAYOUT_TITLE_Y, LAYOUT_TITLE_WIDTH, LAYOUT_TITLE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_ACTIVE_BORDER);
  strncpy_P(b,instructions,sizeof(b));
  displayText(b, LAYOUT_INSTRUCTIONS_TEXT_X, LAYOUT_INSTRUCTIONS_TEXT_Y, LAYOUT_INSTRUCTIONS_TEXT_WIDTH, LAYOUT_INSTRUCTIONS_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  strncpy_P(b,(const char*)F("Push Tune to Save"),sizeof(b));
  displayText(b, LAYOUT_CONFIRM_TEXT_X, LAYOUT_CONFIRM_TEXT_Y, LAYOUT_CONFIRM_TEXT_WIDTH, LAYOUT_CONFIRM_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
}

struct SettingScreen_t {
  const char* const Title;
  const char* const AdditionalText;
  const uint16_t KnobDivider;
  const int16_t StepSize;//int so that it can be negative
  void (*Initialize)(long int* start_value_out);
  void (*Validate)(const long int candidate_value_in, long int* validated_value_out);
  void (*OnValueChange)(const long int new_value, char* buff_out, const size_t buff_out_size);
  void (*Finalize)(const long int final_value);
};

void runSetting(const SettingScreen_t* const p_screen)
{
  SettingScreen_t screen = {nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr};
  memcpy_P(&screen,p_screen,sizeof(screen));
  displayDialog(screen.Title,
                screen.AdditionalText);

  //Wait for button to stop being pressed
  while(btnDown()){
    active_delay(10);
  }
  active_delay(10);

  long int raw_value = 0;
  long int last_value = 0;

  screen.Initialize(&last_value);
  screen.OnValueChange(last_value,b,sizeof(b));
  displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_BACKGROUND);

  raw_value = last_value * (int32_t)screen.KnobDivider;

  while (!btnDown())
  {
    int knob = enc_read();
    if(knob != 0){
      raw_value += knob * screen.StepSize;
    }
    else{
      continue;
    }

    const long int candidate_value = raw_value / (int32_t)screen.KnobDivider;
    long int value = 0;
    screen.Validate(candidate_value,&value);

    //If we're going out of bounds, prevent the raw value from going too far out
    if(candidate_value != value){
      raw_value = value * (int32_t)screen.KnobDivider;
    }

    if(value == last_value){
      continue;
    }
    else{
      screen.OnValueChange(value,b,sizeof(b));
      displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_BACKGROUND);
      last_value = value;
    }
  }

  screen.Finalize(last_value);
}

#define LIMIT(val,min,max) ((val) < (min)) ? (min) : (((max) < (val)) ? (max) : (val))

//Local Oscillator
void ssLocalOscInitialize(long int* start_value_out){
  {
    uint32_t freq = GetActiveVfoFreq();
    freq = (freq/1000L) * 1000L;//round off the current frequency the nearest kHz
    setFrequency(freq);
    si5351bx_setfreq(0, globalSettings.usbCarrierFreq); //set back the carrier oscillator, cw tx switches it off
  }
  *start_value_out = globalSettings.oscillatorCal;
}
void ssLocalOscValidate(const long int candidate_value_in, long int* validated_value_out)
{
  *validated_value_out = candidate_value_in;//No check - allow anything
}
void ssLocalOscChange(const long int new_value, char* buff_out, const size_t buff_out_size)
{
  si5351_set_calibration(new_value);
  setFrequency(GetActiveVfoFreq());
  const long int u = abs(new_value);
  if(new_value != u){
    strncpy_P(buff_out,(const char*)F("-"),buff_out_size);
    ++buff_out;
  }
  formatFreq(u,buff_out,buff_out_size - strlen(buff_out));
  strncat_P(buff_out,(const char*)F("Hz"),buff_out_size - strlen(buff_out));
}
void ssLocalOscFinalize(const long int final_value)
{
  globalSettings.oscillatorCal = final_value;
  SaveSettingsToEeprom();
  si5351_set_calibration(globalSettings.oscillatorCal);
  setFrequency(GetActiveVfoFreq());
}
const char SS_LOCAL_OSC_T [] PROGMEM = "Local Oscillator";
const char SS_LOCAL_OSC_A [] PROGMEM = "Exit menu, tune so that the\ndial displays the desired freq,\nthen tune here until the\nsignal is zerobeat";
const SettingScreen_t ssLocalOsc PROGMEM = {
  SS_LOCAL_OSC_T,
  SS_LOCAL_OSC_A,
  1,
  50,
  ssLocalOscInitialize,
  ssLocalOscValidate,
  ssLocalOscChange,
  ssLocalOscFinalize
};
void runLocalOscSetting(){runSetting(&ssLocalOsc);}

//BFO
void ssBfoInitialize(long int* start_value_out){
  si5351bx_setfreq(0, globalSettings.usbCarrierFreq);
  *start_value_out = globalSettings.usbCarrierFreq;
}
void ssBfoValidate(const long int candidate_value_in, long int* validated_value_out)
{
  *validated_value_out = LIMIT(candidate_value_in,11048000L,11060000L);
}
void ssBfoChange(const long int new_value, char* buff_out, const size_t buff_out_size)
{
  globalSettings.usbCarrierFreq = new_value;
  setFrequency(GetActiveVfoFreq());
  si5351bx_setfreq(0, new_value);
  formatFreq(new_value,buff_out,buff_out_size);
  strncat_P(buff_out,(const char*)F("Hz"),buff_out_size - strlen(buff_out));
}
void ssBfoFinalize(const long int final_value)
{
  globalSettings.usbCarrierFreq = final_value;
  SaveSettingsToEeprom();
  si5351bx_setfreq(0, globalSettings.usbCarrierFreq);
  setFrequency(GetActiveVfoFreq());
}
const char SS_BFO_T [] PROGMEM = "Beat Frequency Osc (BFO)";
const char SS_BFO_A [] PROGMEM = "Exit menu, tune to an unused\nfrequency, then tune here\nuntil the audio is between\n300-3000Hz";
const SettingScreen_t ssBfo PROGMEM = {
  SS_BFO_T,
  SS_BFO_A,
  1,
  -50,//Negative to make dial more intuitive: turning clockwise increases the perceived audio frequency
  ssBfoInitialize,
  ssBfoValidate,
  ssBfoChange,
  ssBfoFinalize
};
void runBfoSetting(){runSetting(&ssBfo);}

//CW Tone
void ssCwToneInitialize(long int* start_value_out)
{
  *start_value_out = globalSettings.cwSideToneFreq;
}
void ssCwToneValidate(const long int candidate_value_in, long int* validated_value_out)
{
  *validated_value_out = LIMIT(candidate_value_in,100,2000);
}
void ssCwToneChange(const long int new_value, char* buff_out, const size_t buff_out_size)
{
  globalSettings.cwSideToneFreq = new_value;
  tone(CW_TONE, globalSettings.cwSideToneFreq);
  ltoa(globalSettings.cwSideToneFreq,buff_out,10);
  strncat_P(buff_out,(const char*)F("Hz"),buff_out_size - strlen(buff_out));
}
void ssCwToneFinalize(const long int final_value)
{
  noTone(CW_TONE);
  globalSettings.cwSideToneFreq = final_value;
  SaveSettingsToEeprom();
}
const char SS_CW_TONE_T [] PROGMEM = "Tone";
const char SS_CW_TONE_A [] PROGMEM = "Select a frequency that\nCW mode to tune for";
const SettingScreen_t ssTone PROGMEM = {
  SS_CW_TONE_T,
  SS_CW_TONE_A,
  1,
  1,
  ssCwToneInitialize,
  ssCwToneValidate,
  ssCwToneChange,
  ssCwToneFinalize
};
void runToneSetting(){runSetting(&ssTone);}

//CW Switch Delay
void ssCwSwitchDelayInitialize(long int* start_value_out)
{
  *start_value_out = globalSettings.cwActiveTimeoutMs;
}
void ssCwSwitchDelayValidate(const long int candidate_value_in, long int* validated_value_out)
{
  *validated_value_out = LIMIT(candidate_value_in,100,1000);
}
void ssCwSwitchDelayChange(const long int new_value, char* buff_out, const size_t buff_out_size)
{
  ltoa(new_value,buff_out,10);
  strncat_P(buff_out,(const char*)F("ms"),buff_out_size - strlen(buff_out));
  morseText(buff_out);
  enc_read();//Consume any rotations during morse playback
}
void ssCwSwitchDelayFinalize(const long int final_value)
{
  globalSettings.cwActiveTimeoutMs = final_value;
  SaveSettingsToEeprom();
}
const char SS_CW_SWITCH_T [] PROGMEM = "Tx to Rx Delay";
const char SS_CW_SWITCH_A [] PROGMEM = "Select how long the radio\nshould wait before switching\nbetween TX and RX when in\nCW mode";
const SettingScreen_t ssCwSwitchDelay PROGMEM = {
  SS_CW_SWITCH_T,
  SS_CW_SWITCH_A,
  1,
  10,
  ssCwSwitchDelayInitialize,
  ssCwSwitchDelayValidate,
  ssCwSwitchDelayChange,
  ssCwSwitchDelayFinalize
};
void runCwSwitchDelaySetting(){runSetting(&ssCwSwitchDelay);}

//CW Keyer
void ssKeyerInitialize(long int* start_value_out)
{
  *start_value_out = globalSettings.keyerMode;
}
void ssKeyerValidate(const long int candidate_value_in, long int* validated_value_out)
{
  *validated_value_out = LIMIT(candidate_value_in,(uint8_t)KeyerMode_e::KEYER_STRAIGHT,(uint8_t)KeyerMode_e::KEYER_IAMBIC_B);
}
void ssKeyerChange(const long int new_value, char* buff_out, const size_t buff_out_size)
{
  char m;
  if(KeyerMode_e::KEYER_STRAIGHT == new_value){
    strncpy_P(buff_out,(const char*)F("Hand Key"),buff_out_size);
    m = 'S';
  }
  else if(KeyerMode_e::KEYER_IAMBIC_A == new_value){
    strncpy_P(buff_out,(const char*)F("Iambic A"),buff_out_size);
    m = 'A';
  }
  else{
    strncpy_P(buff_out,(const char*)F("Iambic B"),buff_out_size);
    m = 'B';
  }
  morseLetter(m);
  enc_read();//Consume any rotations during morse playback
}
void ssKeyerFinalize(const long int final_value)
{
  globalSettings.keyerMode = (KeyerMode_e)final_value;
  SaveSettingsToEeprom();
}
const char SS_KEYER_T [] PROGMEM = "Keyer Type";
const char SS_KEYER_A [] PROGMEM = "Select which type of\nkeyer/paddle is being used";
const SettingScreen_t ssKeyer PROGMEM = {
  SS_KEYER_T,
  SS_KEYER_A,
  10,
  1,
  ssKeyerInitialize,
  ssKeyerValidate,
  ssKeyerChange,
  ssKeyerFinalize
};
void runKeyerSetting(){runSetting(&ssKeyer);}

//Morse menu playback
void ssMorseMenuInitialize(long int* start_value_out)
{
  *start_value_out = globalSettings.morseMenuOn;
}
void ssMorseMenuValidate(const long int candidate_value_in, long int* validated_value_out)
{
  *validated_value_out = LIMIT(candidate_value_in,0,1);
}
void ssMorseMenuChange(const long int new_value, char* buff_out, const size_t buff_out_size)
{
  char m;
  if(new_value){
    strncpy_P(buff_out,(const char*)F("Yes"),buff_out_size);
    m = 'Y';
  }
  else{
    strncpy_P(buff_out,(const char*)F("No"),buff_out_size);
    m = 'N';
  }
  morseLetter(m);
  enc_read();//Consume any rotations during morse playback
}
void ssMorseMenuFinalize(const long int final_value)
{
  globalSettings.morseMenuOn = final_value;
  SaveSettingsToEeprom();
}
const char SS_MORSE_MENU_T [] PROGMEM = "Menu Audio";
const char SS_MORSE_MENU_A [] PROGMEM = "When on, menu selections\nwill play morse code";
const SettingScreen_t ssMorseMenu PROGMEM = {
  SS_MORSE_MENU_T,
  SS_MORSE_MENU_A,
  10,
  1,
  ssMorseMenuInitialize,
  ssMorseMenuValidate,
  ssMorseMenuChange,
  ssMorseMenuFinalize
};
void runMorseMenuSetting(){runSetting(&ssMorseMenu);}

//CW Speed
void ssCwSpeedInitialize(long int* start_value_out)
{
  *start_value_out = 1200L/globalSettings.cwDitDurationMs;
}
void ssCwSpeedValidate(const long int candidate_value_in, long int* validated_value_out)
{
  *validated_value_out = LIMIT(candidate_value_in,1,100);
}
void ssCwSpeedChange(const long int new_value, char* buff_out, const size_t /*buff_out_size*/)
{
  ltoa(new_value, buff_out, 10);
  morseText(buff_out,1200L/new_value);
  enc_read();//Consume any rotations during morse playback
}
void ssCwSpeedFinalize(const long int final_value)
{
  globalSettings.cwDitDurationMs = 1200L/final_value;
  SaveSettingsToEeprom();
}
const char SS_CW_SPEED_T [] PROGMEM = "Play Speed";
const char SS_CW_SPEED_A [] PROGMEM = "Select speed to play CW\ncharacters";
const SettingScreen_t ssCwSpeed PROGMEM = {
  SS_CW_SPEED_T,
  SS_CW_SPEED_A,
  5,
  1,
  ssCwSpeedInitialize,
  ssCwSpeedValidate,
  ssCwSpeedChange,
  ssCwSpeedFinalize
};
void runCwSpeedSetting(){runSetting(&ssCwSpeed);}

//Reset all settings
void ssResetAllInitialize(long int* start_value_out)
{
  *start_value_out = 0;//Default to NOT resetting
}
void ssResetAllValidate(const long int candidate_value_in, long int* validated_value_out)
{
  *validated_value_out = LIMIT(candidate_value_in,0,1);
}
void ssResetAllChange(const long int new_value, char* buff_out, const size_t buff_out_size)
{
  char m;
  if(new_value){
    strncpy_P(buff_out,(const char*)F("Yes"),buff_out_size);
    m = 'Y';
  }
  else{
    strncpy_P(buff_out,(const char*)F("No"),buff_out_size);
    m = 'N';
  }
  morseLetter(m);
  enc_read();//Consume any rotations during morse playback
}
void ssResetAllFinalize(const long int final_value)
{
  if(final_value){
    LoadDefaultSettings();
    SaveSettingsToEeprom();
    setup();
  }
}
const char SS_RESET_ALL_T [] PROGMEM = "Reset All";
const char SS_RESET_ALL_A [] PROGMEM = "WARNING: Selecting \"Yes\"\nwill reset all calibrations and\nsettings to their default\nvalues";
const SettingScreen_t ssResetAll PROGMEM = {
  SS_RESET_ALL_T,
  SS_RESET_ALL_A,
  20,
  1,
  ssResetAllInitialize,
  ssResetAllValidate,
  ssResetAllChange,
  ssResetAllFinalize
};
void runResetAllSetting(){runSetting(&ssResetAll);}

struct MenuItem_t {
  const char* const ItemName;
  void (*OnSelect)();
};

void runMenu(const MenuItem_t* const menu_items, const uint16_t num_items);
#define RUN_MENU(menu) runMenu(menu,sizeof(menu)/sizeof(menu[0]))

const char MT_CAL [] PROGMEM = "Calibrations";
const char MI_TOUCH [] PROGMEM = "Touch Screen";
const MenuItem_t calibrationMenu [] PROGMEM {
  {MT_CAL,nullptr},//Title
  {SS_LOCAL_OSC_T,runLocalOscSetting},
  {SS_BFO_T,runBfoSetting},
  {MI_TOUCH,setupTouch},
};
void runCalibrationMenu(){RUN_MENU(calibrationMenu);}

const char MT_CW [] PROGMEM = "CW Setup";
const MenuItem_t cwMenu [] PROGMEM {
  {MT_CW,nullptr},//Title
  {SS_CW_TONE_T,runToneSetting},
  {SS_CW_SWITCH_T,runCwSwitchDelaySetting},
  {SS_KEYER_T,runKeyerSetting},
  {SS_MORSE_MENU_T,runMorseMenuSetting},
  {SS_CW_SPEED_T,runCwSpeedSetting},
};
void runCwMenu(){RUN_MENU(cwMenu);}

const char MT_SETTINGS [] PROGMEM = "Settings";
const MenuItem_t mainMenu [] PROGMEM {
  {MT_SETTINGS,nullptr},//Title
  {MT_CAL,runCalibrationMenu},
  {MT_CW,runCwMenu},
  {SS_RESET_ALL_T,runResetAllSetting},
};

const char MI_EXIT [] PROGMEM = "Exit";
const MenuItem_t exitMenu PROGMEM = {MI_EXIT,nullptr};

void drawMenu(const MenuItem_t* const items, const uint16_t num_items)
{
  displayClear(COLOR_BACKGROUND);
  MenuItem_t mi = {"",nullptr};
  memcpy_P(&mi,&items[0],sizeof(mi));
  strncpy_P(b,mi.ItemName,sizeof(b));
  displayText(b, LAYOUT_TITLE_X, LAYOUT_TITLE_Y, LAYOUT_TITLE_WIDTH, LAYOUT_TITLE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_ACTIVE_BORDER);
  for(unsigned int i = 1; i < num_items; ++i){
    memcpy_P(&mi,&items[i],sizeof(mi));
    strncpy_P(b,mi.ItemName,sizeof(b));
    displayText(b, LAYOUT_ITEM_X, LAYOUT_ITEM_Y + (i-1)*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_INACTIVE_BORDER);
  }
  memcpy_P(&mi,&exitMenu,sizeof(mi));
  strncpy_P(b,mi.ItemName,sizeof(b));
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
  const int MAX_KNOB_VALUE = num_items*COUNTS_PER_ITEM - 1;
  int knob_sum = 0;
  unsigned int old_index = -1;

  drawMenu(menu_items,num_items);
  movePuck(-1,0);//Force draw of puck

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

    if(globalSettings.morseMenuOn //Only spend cycles copying menu item into RAM if we actually need to
     && (old_index != index)){
      if(num_items-1 > index){
        MenuItem_t mi = {"",nullptr};
        memcpy_P(&mi,&menu_items[index+1],sizeof(mi));//The 0th element in the array is the title, so offset by 1
        strncpy_P(b,mi.ItemName,sizeof(b));
      }
      else{
        strncpy_P(b,MI_EXIT,sizeof(b));
      }
      morseText(b);
      enc_read();//Consume any rotations during morse playback
    }

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
