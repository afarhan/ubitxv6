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
 
void drawCommandbar(char *text){
  utftFillrect(30,45,280, 32, ILI9341_NAVY);
  displayRawText(text, 30, 45, ILI9341_WHITE, ILI9341_NAVY);
}

/** A generic control to read variable values
*/
int getValueByKnob(int minimum, int maximum, int step_size,  int initial, char* prefix, char *postfix)
{
    int knob = 0;
    int knob_value;

    while (btnDown())
      active_delay(100);

    active_delay(200);
    knob_value = initial;
     
    strcpy(b, prefix);
    itoa(knob_value, c, 10);
    strcat(b, c);
    strcat(b, postfix);
    drawCommandbar(b);
    //printLine2(b);
    //active_delay(300);

    while(!btnDown() && digitalRead(PTT) == HIGH){

      knob = enc_read();
      if (knob != 0){
        if (knob_value > minimum && knob < 0)
          knob_value -= step_size;
        if (knob_value < maximum && knob > 0)
          knob_value += step_size;
          
        //printLine2(prefix);
        strcpy(b, prefix);
        itoa(knob_value, c, 10);
        strcat(b, c);
        strcat(b, postfix);
        drawCommandbar(b);
//        utftFillrect(30,41,280, 32, ILI9341_NAVY);
//        displayRawText(b, 30, 41, ILI9341_WHITE, ILI9341_NAVY);
        //printLine1(b);
      }
      checkCAT();
    }
   utftFillrect(30,41,280, 32, ILI9341_NAVY);
   return knob_value;
}

// The generic routine to display one line on the LCD 
void printLine(int linenmbr, char *c) {
  if (strcmp(c, printBuff[linenmbr])) {     // only refresh the display when there was a change
    utftFillrect(50,  100 + (50 * linenmbr), 200, 40, ILI9341_BLACK);
    displayRawText(c, 50,  100 + (50 * linenmbr), ILI9341_WHITE, ILI9341_BLACK);
    strcpy(printBuff[linenmbr], c);
  }
}

//  short cut to print to the first line
void printLine1(char *c){
  printLine(1,c);
  //displayText(c, 0,180,320, 25, ILI9341_YELLOW, ILI9341_BLACK, ILI9341_BLUE);
}

//  short cut to print to the first line
void printLine2(char *c){
  printLine(0,c);

  //displayText(c, 0,210,320, 25, ILI9341_YELLOW, ILI9341_BLACK, ILI9341_BLUE);
}

void printFLine1(String s){
  printLine1(s.c_str());
}

void printFLine2(String s){
  printLine2(s.c_str());
}

void setupExit(){
  menuOn = 0;
  guiUpdate();
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
  displayText(c, 110, 100, 100, 30, ILI9341_CYAN, ILI9341_NAVY, ILI9341_NAVY);
}

void displayDialog(char *title, char *instructions){
  displayClear(ILI9341_BLACK);
  utftRect(10,10,300,220, ILI9341_WHITE);
  utftHline(20,45,280,ILI9341_WHITE);
  utftRect(12,12,296,216, ILI9341_WHITE);
  displayRawText(title, 20, 20, ILI9341_CYAN, ILI9341_NAVY);
  displayRawText(instructions, 20, 200, ILI9341_CYAN, ILI9341_NAVY);
}

 //this is used by the si5351 routines in the ubitx_5351 file
extern int32_t calibration;
extern uint32_t si5351bx_vcoa;

int setupFreq(){
  int knob = 0;
  int32_t prev_calibration;

  displayDialog("Set Frequency", "Push TUNE to Save"); 

  //round off the the nearest khz
  frequency = (frequency/1000l)* 1000l;
  setFrequency(frequency);
  
  displayRawText("You should have a", 20, 50, ILI9341_CYAN, ILI9341_NAVY);
  displayRawText("signal exactly at ", 20, 75, ILI9341_CYAN, ILI9341_NAVY);
  ltoa(c, frequency/1000l, 10);

  displayRawText(c, 20, 100, ILI9341_CYAN, ILI9341_NAVY);
  displayRawText("Rotate to zerobeat", 20, 180, ILI9341_CYAN, ILI9341_NAVY);
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

    //displayRawText("Rotate to zerobeat", 20, 120, ILI9341_CYAN, ILI9341_NAVY);
    
    ltoa(calibration, b, 10);
    displayText(b, 100, 140, 100, 26, ILI9341_CYAN, ILI9341_NAVY, ILI9341_WHITE);
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
  printLine2("");
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
  displayText(b, 100, 100, 120, 26, ILI9341_CYAN, ILI9341_BLACK, ILI9341_BLACK);

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
    displayText(b, 100, 100, 120, 26, ILI9341_CYAN, ILI9341_BLACK, ILI9341_BLACK);
      
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
    displayText("< Hand Key >", 100, 100, 120, 26, ILI9341_CYAN, ILI9341_BLACK, ILI9341_BLACK);
  else if (keyerControl & IAMBICB)
    displayText("< Iambic A >", 100, 100, 120, 26, ILI9341_CYAN, ILI9341_BLACK, ILI9341_BLACK);
  else 
    displayText("< Iambic B >", 100, 100, 120, 26, ILI9341_CYAN, ILI9341_BLACK, ILI9341_BLACK);

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
      displayText("< Hand Key >", 100, 100, 120, 26, ILI9341_CYAN, ILI9341_BLACK, ILI9341_BLACK);
    else if (tmp_key == 1)
      displayText("< Iambic A >", 100, 100, 120, 26, ILI9341_CYAN, ILI9341_BLACK, ILI9341_BLACK);
    else if (tmp_key == 2)
      displayText("< Iambic B >", 100, 100, 120, 26, ILI9341_CYAN, ILI9341_BLACK, ILI9341_BLACK);
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
  displayClear(ILI9341_BLACK);
 
  displayText("Setup", 10, 10, 300, 35, ILI9341_WHITE, ILI9341_NAVY, ILI9341_WHITE); 
  utftRect(10,10,300,220, ILI9341_WHITE);
  
  displayRawText("Set Freq...", 30, 50, ILI9341_WHITE, ILI9341_NAVY);       
  displayRawText("Set BFO...", 30, 80, ILI9341_WHITE, ILI9341_NAVY);       
  displayRawText("CW Delay...", 30, 110, ILI9341_WHITE, ILI9341_NAVY);       
  displayRawText("CW Keyer...", 30, 140, ILI9341_WHITE, ILI9341_NAVY);       
  displayRawText("Touch Screen...", 30, 170, ILI9341_WHITE, ILI9341_NAVY);       
  displayRawText("Exit", 30, 200, ILI9341_WHITE, ILI9341_NAVY);       
}

static int prevPuck = -1;
void movePuck(int i){
  if (prevPuck >= 0)
    utftRect(15, 49 + (prevPuck * 30), 290, 25, ILI9341_NAVY);
  utftRect(15, 49 + (i * 30), 290, 25, ILI9341_WHITE);
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
      setupExit();
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
