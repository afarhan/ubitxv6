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

void setupFreq(){
  int knob = 0;
  int32_t prev_calibration;

  displayDialog("Set Frequency", "Push TUNE to Save"); 

  //round off the the nearest khz
  frequency = (frequency/1000l)* 1000l;
  setFrequency(frequency);
  
  displayRawText("You should have a", 20, 50, DISPLAY_CYAN, DISPLAY_NAVY);
  displayRawText("signal exactly at ", 20, 75, DISPLAY_CYAN, DISPLAY_NAVY);
  ltoa(frequency/1000l, c, 10);
  strcat(c, " KHz");
  displayRawText(c, 20, 100, DISPLAY_CYAN, DISPLAY_NAVY);

  displayRawText("Rotate to zerobeat", 20, 180, DISPLAY_CYAN, DISPLAY_NAVY);
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
    displayText(b, 100, 140, 100, 26, DISPLAY_CYAN, DISPLAY_NAVY, DISPLAY_WHITE);
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
  displayText(b, 100, 100, 120, 26, DISPLAY_CYAN, DISPLAY_BLACK, DISPLAY_BLACK);

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
    displayText(b, 100, 100, 120, 26, DISPLAY_CYAN, DISPLAY_BLACK, DISPLAY_BLACK);
      
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
    displayText("< Hand Key >", 100, 100, 120, 26, DISPLAY_CYAN, DISPLAY_BLACK, DISPLAY_BLACK);
  else if (keyerControl & IAMBICB)
    displayText("< Iambic A >", 100, 100, 120, 26, DISPLAY_CYAN, DISPLAY_BLACK, DISPLAY_BLACK);
  else 
    displayText("< Iambic B >", 100, 100, 120, 26, DISPLAY_CYAN, DISPLAY_BLACK, DISPLAY_BLACK);

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
      displayText("< Hand Key >", 100, 100, 120, 26, DISPLAY_CYAN, DISPLAY_BLACK, DISPLAY_BLACK);
    else if (tmp_key == 1)
      displayText("< Iambic A >", 100, 100, 120, 26, DISPLAY_CYAN, DISPLAY_BLACK, DISPLAY_BLACK);
    else if (tmp_key == 2)
      displayText("< Iambic B >", 100, 100, 120, 26, DISPLAY_CYAN, DISPLAY_BLACK, DISPLAY_BLACK);
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

void drawSetupMenu(){
  displayClear(DISPLAY_BLACK);
 
  displayText("Setup", 10, 10, 300, 35, DISPLAY_WHITE, DISPLAY_NAVY, DISPLAY_WHITE); 
  displayRect(10,10,300,220, DISPLAY_WHITE);
  
  displayRawText("Set Freq...", 30, 50, DISPLAY_WHITE, DISPLAY_NAVY);       
  displayRawText("Set BFO...", 30, 80, DISPLAY_WHITE, DISPLAY_NAVY);       
  displayRawText("CW Delay...", 30, 110, DISPLAY_WHITE, DISPLAY_NAVY);       
  displayRawText("CW Keyer...", 30, 140, DISPLAY_WHITE, DISPLAY_NAVY);       
  displayRawText("Touch Screen...", 30, 170, DISPLAY_WHITE, DISPLAY_NAVY);       
  displayRawText("Exit", 30, 200, DISPLAY_WHITE, DISPLAY_NAVY);       
}

static int prevPuck = -1;
void movePuck(int i){
  if (prevPuck >= 0)
    displayRect(15, 49 + (prevPuck * 30), 290, 25, DISPLAY_NAVY);
  displayRect(15, 49 + (i * 30), 290, 25, DISPLAY_WHITE);
  prevPuck = i;
  
}

void doSetup2(){
  int select=0, i,btnState;

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
      if (select + i < 60)
        select += i;
        movePuck(select/10);
    }
    if (i < 0 && select - i >= 0){
      select += i;      //caught ya, i is already -ve here, so you add it
      movePuck(select/10);
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
    
    if (select < 10)
     setupFreq();
    else if (select < 20 )
      setupBFO(); 
    else if (select < 30 )
      setupCwDelay(); 
    else if (select < 40)
        setupKeyer();
    else if (select < 50)
        setupTouch();
    else
      break; //exit setup was chosen
      //setupExit();
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
