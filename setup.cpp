#include <Arduino.h>
#include "colors.h"
#include "encoder.h"
#include "menu.h"
#include "morse.h"
#include "nano_gui.h"
#include "pin_definitions.h"
#include "scratch_space.h"
#include "setup.h"
#include "settings.h"
#include "si5351.h"
#include "tuner.h"
#include "utils.h"

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

static const unsigned int LAYOUT_SETTING_REF_VALUE_X = LAYOUT_ITEM_X;
static const unsigned int LAYOUT_SETTING_REF_VALUE_Y = LAYOUT_ITEM_Y + 3*LAYOUT_ITEM_PITCH_Y;
static const unsigned int LAYOUT_SETTING_REF_VALUE_WIDTH = LAYOUT_ITEM_WIDTH;
static const unsigned int LAYOUT_SETTING_REF_VALUE_HEIGHT = LAYOUT_ITEM_HEIGHT;

static const unsigned int LAYOUT_SETTING_VALUE_X = LAYOUT_ITEM_X;
static const unsigned int LAYOUT_SETTING_VALUE_Y = LAYOUT_ITEM_Y + 4*LAYOUT_ITEM_PITCH_Y;
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
  displayText(b, LAYOUT_INSTRUCTIONS_TEXT_X, LAYOUT_INSTRUCTIONS_TEXT_Y, LAYOUT_INSTRUCTIONS_TEXT_WIDTH, LAYOUT_INSTRUCTIONS_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND, TextJustification_e::Left);
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

void drawSetting(const SettingScreen_t* const screen)
{
  displayDialog(screen->Title,
                screen->AdditionalText);
}

//State variables for settings
int32_t setupMenuRawValue = 0;
int32_t setupMenuLastValue = 0;
const SettingScreen_t* activeSettingP;

void activateSetting(SettingScreen_t* new_setting_P);

void initSetting();
MenuReturn_e runSetting(const ButtonPress_e tuner_button,
                        const ButtonPress_e touch_button,
                        const Point touch_point,
                        const int16_t knob);
Menu_t setupMenuActiveSettingMenu = {
  initSetting,
  runSetting,
  nullptr
};

void initSetting()
{
  if(nullptr == activeSettingP){
    return;
  }
  SettingScreen_t screen = {nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr};
  memcpy_P(&screen,activeSettingP,sizeof(screen));
  drawSetting(&screen);
  screen.Initialize(&setupMenuLastValue);
  screen.OnValueChange(setupMenuLastValue,b,sizeof(b));
  displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_BACKGROUND);
  displayText(b, LAYOUT_SETTING_REF_VALUE_X, LAYOUT_SETTING_REF_VALUE_Y, LAYOUT_SETTING_REF_VALUE_WIDTH, LAYOUT_SETTING_REF_VALUE_HEIGHT, COLOR_SETTING_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND);
  setupMenuRawValue = setupMenuLastValue * (int32_t)screen.KnobDivider;
}

MenuReturn_e runSetting(const ButtonPress_e tuner_button,
                        const ButtonPress_e touch_button,
                        const Point touch_point,
                        const int16_t knob)
{
  if(nullptr == activeSettingP){
    return MenuReturn_e::ExitedRedraw;
  }
  SettingScreen_t screen = {nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr};
  memcpy_P(&screen,activeSettingP,sizeof(screen));

  if(ButtonPress_e::NotPressed != tuner_button){
    //Long or short press, we do the same thing
    screen.Finalize(setupMenuLastValue);
    return MenuReturn_e::ExitedRedraw;
  }

  (void)touch_button;(void)touch_point;//TODO: handle touch input?

  if(0 != knob){
    setupMenuRawValue += knob * screen.StepSize;

    const int32_t candidate_value = setupMenuRawValue / (int32_t)screen.KnobDivider;
    int32_t value = 0;
    screen.Validate(candidate_value,&value);

    //If we're going out of bounds, prevent the raw value from going too far out
    if(candidate_value != value){
      setupMenuRawValue = value * (int32_t)screen.KnobDivider;
    }

    if(value != setupMenuLastValue){
      screen.OnValueChange(value,b,sizeof(b));
      displayText(b, LAYOUT_SETTING_VALUE_X, LAYOUT_SETTING_VALUE_Y, LAYOUT_SETTING_VALUE_WIDTH, LAYOUT_SETTING_VALUE_HEIGHT, COLOR_TEXT, COLOR_TITLE_BACKGROUND, COLOR_BACKGROUND);
      setupMenuLastValue = value;
    }
  }

  return MenuReturn_e::StillActive;
}

void activateSetting(const SettingScreen_t *const new_setting_P)
{
  activeSettingP = new_setting_P;
  enterSubmenu(&setupMenuActiveSettingMenu);
}

//Local Oscillator
void ssLocalOscInitialize(long int* start_value_out){
  {
    uint32_t freq = GetActiveVfoFreq();
    freq = (freq/1000L) * 1000L;//round off the current frequency the nearest kHz
    setFrequency(freq);
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
void runLocalOscSetting(){activateSetting(&ssLocalOsc);}

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
void runBfoSetting(){activateSetting(&ssBfo);}

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
  tone(PIN_CW_TONE, globalSettings.cwSideToneFreq);
  ltoa(globalSettings.cwSideToneFreq,buff_out,10);
  strncat_P(buff_out,(const char*)F("Hz"),buff_out_size - strlen(buff_out));
}
void ssCwToneFinalize(const long int final_value)
{
  noTone(PIN_CW_TONE);
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
void runToneSetting(){activateSetting(&ssTone);}

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
void runCwSwitchDelaySetting(){activateSetting(&ssCwSwitchDelay);}

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
void runKeyerSetting(){activateSetting(&ssKeyer);}

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
void runMorseMenuSetting(){activateSetting(&ssMorseMenu);}

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
void runCwSpeedSetting(){activateSetting(&ssCwSpeed);}

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
void runResetAllSetting(){activateSetting(&ssResetAll);}

struct MenuItem_t {
  const char* const ItemName;
  void (*OnSelect)();
};

void initSetupMenu(const MenuItem_t* const menu_items,
                   const uint16_t num_items);
MenuReturn_e runSetupMenu(const MenuItem_t* const menu_items,
                          const uint16_t num_items,
                          const ButtonPress_e tuner_button,
                          const ButtonPress_e touch_button,
                          const Point touch_point,
                          const int16_t knob);

#define GENERATE_MENU_T(menu_name) \
          void initSetupMenu##menu_name(void)\
          {\
            initSetupMenu(menuItems##menu_name,sizeof(menuItems##menu_name)/sizeof(menuItems##menu_name[0]));\
          }\
          MenuReturn_e runSetupMenu##menu_name(const ButtonPress_e tuner_button,\
                                            const ButtonPress_e touch_button,\
                                            const Point touch_point,\
                                            const int16_t knob)\
          {\
            return runSetupMenu(menuItems##menu_name,\
                                sizeof(menuItems##menu_name)/sizeof(menuItems##menu_name[0]),\
                                tuner_button,\
                                touch_button,\
                                touch_point,\
                                knob\
                                );\
          }\
          Menu_t setupMenu##menu_name = {\
            initSetupMenu##menu_name,\
            runSetupMenu##menu_name,\
            nullptr\
          };\
          void run##menu_name##Menu(void)\
          {\
            enterSubmenu(&setupMenu##menu_name);\
          }\

const char MT_CAL [] PROGMEM = "Calibrations";
const char MI_TOUCH [] PROGMEM = "Touch Screen";
const MenuItem_t menuItemsCalibration [] PROGMEM {
  {MT_CAL,nullptr},//Title
  {SS_LOCAL_OSC_T,runLocalOscSetting},
  {SS_BFO_T,runBfoSetting},
  {MI_TOUCH,setupTouch},
};
GENERATE_MENU_T(Calibration);

const char MT_CW [] PROGMEM = "CW Setup";
const MenuItem_t menuItemsCw [] PROGMEM {
  {MT_CW,nullptr},//Title
  {SS_CW_TONE_T,runToneSetting},
  {SS_CW_SWITCH_T,runCwSwitchDelaySetting},
  {SS_KEYER_T,runKeyerSetting},
  {SS_MORSE_MENU_T,runMorseMenuSetting},
  {SS_CW_SPEED_T,runCwSpeedSetting},
};
GENERATE_MENU_T(Cw);

const char MT_SETTINGS [] PROGMEM = "Settings";
const MenuItem_t menuItemsSetupRoot [] PROGMEM {
  {MT_SETTINGS,nullptr},//Title
  {MT_CAL,runCalibrationMenu},
  {MT_CW,runCwMenu},
  {SS_RESET_ALL_T,runResetAllSetting},
};
GENERATE_MENU_T(SetupRoot);
Menu_t *const setupMenu = &setupMenuSetupRoot;

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
    displayText(b, LAYOUT_ITEM_X, LAYOUT_ITEM_Y + (i-1)*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_INACTIVE_BORDER, TextJustification_e::Left);
  }
  memcpy_P(&mi,&exitMenu,sizeof(mi));
  strncpy_P(b,mi.ItemName,sizeof(b));
  displayText(b, LAYOUT_ITEM_X, LAYOUT_ITEM_Y + (num_items-1)*LAYOUT_ITEM_PITCH_Y, LAYOUT_ITEM_WIDTH, LAYOUT_ITEM_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_INACTIVE_BORDER, TextJustification_e::Left);
}

void movePuck(unsigned int old_index,
              unsigned int new_index)
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

int16_t setupMenuSelector = 0;

void initSetupMenu(const MenuItem_t* const menu_items,
                   const uint16_t num_items)
{
  drawMenu(menu_items,num_items);
  setupMenuSelector = 0;
  movePuck(-1,0);//Force draw of puck
}

MenuReturn_e runSetupMenu(const MenuItem_t* const menu_items,
                          const uint16_t num_items,
                          const ButtonPress_e tuner_button,
                          const ButtonPress_e touch_button,
                          const Point touch_point,
                          const int16_t knob)
{
  const int16_t cur_index = setupMenuSelector/MENU_KNOB_COUNTS_PER_ITEM;
  const int16_t exit_index = num_items - 1;

  if(ButtonPress_e::NotPressed != tuner_button){
    //Don't care what kind of press
    if(exit_index <= cur_index){
      return MenuReturn_e::ExitedRedraw;
    }

    MenuItem_t mi = {"",nullptr};
    memcpy_P(&mi,&menu_items[cur_index+1],sizeof(mi));//The 0th element in the array is the title, so offset by 1
    mi.OnSelect();
  }

  (void)touch_button;(void)touch_point;//TODO: handle touch input?

  if(0 != knob){
    setupMenuSelector = LIMIT(setupMenuSelector + knob,0,(int16_t)(num_items*MENU_KNOB_COUNTS_PER_ITEM - 1));
    const int16_t new_index = setupMenuSelector/MENU_KNOB_COUNTS_PER_ITEM;
    if(cur_index != new_index){
      movePuck(cur_index,new_index);
      if(globalSettings.morseMenuOn){//Only spend cycles copying menu item into RAM if we actually need to
        if(exit_index <= cur_index){
          strncpy_P(b,MI_EXIT,sizeof(b));
        }
        else{
          MenuItem_t mi = {"",nullptr};
          memcpy_P(&mi,&menu_items[cur_index+1],sizeof(mi));//The 0th element in the array is the title, so offset by 1
          strncpy_P(b,mi.ItemName,sizeof(b));
        }
        morseText(b);
      }
    }
  }

  return MenuReturn_e::StillActive;
}
