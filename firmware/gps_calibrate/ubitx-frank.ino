/*

 Arduino Controlled GPS Corrected Dual Output Si5351A VFO
 
 A VFO project that uses an Arduino Uno or Nano to control a SI5351A clock 
 generator breakout board. The VFO may be used in stand-alone mode or with 
 a GPS receiver to software calibrate the output frequency.
 
 Permission is granted to use, copy, modify, and distribute this software
 and documentation for non-commercial purposes.
 
 Copyright (C) 2014,  Gene Marcus W3PM GM4YRE
 
 16 December,2014
 
 v5     12 July, 2015    Corrected band select algorithm
 v5_1   13 July, 2015    Changed "KHz" to "kHZ" in the LCD frequency display
 v5_2   20 September,    2016 Modified debounce function to be compatible with 
                         Arduino IDE 1.6.11
 v5_3   28 January,2017  Modified coding to enhance 9600 operation. The modifcation
                         is courtesy of Jan, PE1CID.
     
                                        
 
 Acknowledgements: 

 Special thanks to Jan, PE1CID for his effort to streamline the code and improve 
 operation at 9600 baud. Portions of the GPS receive code were influenced by 
 Igor Gonzalez Martin's Arduino tutorial. Thanks to N2END for finding compatibility 
 bug with Arduino 1.6.11 and testing revision v5_2.
 
 
 ------------------------------------------------------------------------
 Nano Digital Pin Allocation
 
 D0/RX  GPS RX (this pin may differ on the Nano board)
 D1  
 D2  GPS 1pps input 
 D3  Rotary encoder pin A
 D4  Rotary encoder pin B
 D5  2.5 MHz input from Si5351 CLK0 pin
 D6  Frequency resolution button
 D7  LCD RS 
 D8  LCD enable
 D9  LCD DB4
 D10 LCD DB5
 D11 LCD DB6
 D12 LCD DB7
 D13 
 A0/D14 Decrease frequency button
 A1/D15 Increase frequency button
 A2/D16 Offset enable
 A3/D17 Band Select button               
 A4/D18 Si5351 SDA
 A5/D19 Si5351 SCL 
 
 ----------------------------------------------------------------
 */

/*
_________________________________________________________________
 Band Select frequency format:
 
 Column 1 = CLK1 VFO start frequency in Hz
 Column 2 = CLK2 LO frequency in Hz
 Column 3 = LCD display arithmetic operation 
 0 = no change
 1 = Add CLK1 and CLK2
 2 = Subtract CLK1 from CLK2  
 
 Example: {5286500,8998500,1}, will result in
 LCD display: 14.285000 MHz
 CLK1 output: 5.286500 MHz 
 CLK2 output: 8.998500 MHz
 
 Enter any number of Band Select frequencies.  
 Use (0,0,0) as the last entry.    
 
 Restrict frequency entries to > 1 MHz and < 80 MHz
 
 ___________Enter Band Select frequencies below_____________________
 */
const unsigned long Freq_array [] [3] = {
  {  5000000,0,0      },            //
  {  6250000,0,0      },
  {  7500000,0,0      },            
  (0,0,0)
  };


//_________________________Enter offset frequency (Hz) below:__________________________________
int fOffset = -600;        // -600 Hz offset              


//_________________________GPS 1pps calibrate? (0 = NO, 1 = YES)_________________________________
int GPSflag = 1;


//___________________________Enter stand-alone calibration factor:______________________________
//    This entry is not required if GPS calibration is selected.
//  - Connect VFO to a frequency counter
//  - Set VFO to 25 MHz
//  - Annotate counter frequency in Hz
//  - Subtract 25 MHz from counter reading 
//  - Enter the difference in Hz (i.e. -245)
int CalFactor = 0;


// include the library code:
#include <LiquidCrystal.h>
#include <string.h>
#include <ctype.h>
#include <avr/interrupt.h>  
#include <avr/io.h>
#include <Wire.h>

// Set up MCU pins
#define ppsPin                   2 
#define encoderPinA              3 
#define encoderPinB              4
#define Resolution               6
#define RS                       7 
#define E                        8 
#define DB4                      9
#define DB5                     10 
#define DB6                     11 
#define DB7                     12
#define FreqDown                A0
#define FreqUp                  A1
#define Offset                  A2
#define BandSelect              A3

// Set sI5351A I2C address
#define Si5351A_addr          0x60 

// Define Si5351A register addresses
#define CLK_ENABLE_CONTROL       3
#define CLK0_CONTROL            16 
#define CLK1_CONTROL            17
#define CLK2_CONTROL            18
#define SYNTH_PLL_A             26
#define SYNTH_PLL_B             34
#define SYNTH_MS_0              42
#define SYNTH_MS_1              50
#define SYNTH_MS_2              58
#define CLK0_PHOFF             165
#define CLK1_PHOFF             166
#define CLK2_PHOFF             167 
#define PLL_RESET              177
#define XTAL_LOAD_CAP          183

// Define output multisynth divisors:
#define DIV_GPS_COUNTER        320
#define DIV_50M                600/5

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(RS, E, DB4, DB5, DB6, DB7);

// configure variables 
byte fStepcount,offsetFlag=0,band;
String resolution = "1 Hz  ";
char GPSlocator[7],StartCommand[7] = "$GPGGA",StartCommand2[7] = "$GPRMC",buffer[300] = "",EW,NS;
int IndiceCount=0,StartCount=0,counter=0,indices[13];
int Lat10,Lat1,Lon100,Lon10,Lon1,validGPSflag;
int dLon1,dLon2,dLat1,dLat2,dlon;
int byteGPS=-1,second=0,minute=0,hour=0;
unsigned int tcount=2,encoderA,encoderB,encoderC=1;
unsigned long time,fStep=1,XtalFreq=25000000;
unsigned long mult=0,Freq_1,Freq_2,debounce,DebounceDelay=500000;

static bool s_GpsOneSecTick = false; 

//******************************************************************
// Clock - interrupt routine used as master timekeeper
// Called every second by GPS 1PPS on MCU pin 2
//******************************************************************
void PPSinterrupt()
{
  s_GpsOneSecTick = true;                        // New second by GPS.

  tcount++;
  if (tcount == 4)                               // Start counting the 2.5 MHz signal from Si5351A CLK0
  {
    TCCR1B = 7;                                  //Clock on rising edge of pin 5
  }
  else if (tcount == 44)                         //The 40 second gate time elapsed - stop counting
  {     
    TCCR1B = 0;                                  //Turn off counter
    XtalFreq = mult * 0x10000 + TCNT1;           //Calculate correction factor
    TCNT1 = 0;                                   //Reset count to zero
    mult = 0;
    tcount = 0;                                  //Reset the seconds counter
  }
}

// Timer 1 overflow intrrupt vector.
ISR(TIMER1_OVF_vect) 
{
  mult++;                                          //Increment multiplier
  TIFR1 = (1<<TOV1);                               //Clear overlow flag 
}


void setup()
{

  Wire.begin(1);                  // join I2C bus (address = 1)
  si5351aStart();

  //Set up Timer1 as a frequency counter - input at pin 5
  TCCR1B = 0;                                    //Disable Timer5 during setup
  TCCR1A = 0;                                    //Reset
  TCNT1  = 0;                                    //Reset counter to zero
  TIFR1  = 1;                                    //Reset overflow
  TIMSK1 = 1;                                    //Turn on overflow flag

  // Inititalize GPS 1pps input
  pinMode(ppsPin, INPUT);

  // Set 1PPS pin 2 for external interrupt input
  attachInterrupt(digitalPinToInterrupt(ppsPin), PPSinterrupt, RISING);  

  // Add CalFactor to the Si5351 crystal frequency for non-GPS frequency updates
  if(GPSflag == 0)
  {
    XtalFreq += CalFactor;
    detachInterrupt(digitalPinToInterrupt(ppsPin)); // Disable the 1pps interrupt
  } 

  // Make XtalFreq compatable with GPS correction variable
  XtalFreq *= 4;

  // set up the LCD's number of columns and rows 
  lcd.begin(16,2); 

  // Set up rotary encoder
  pinMode(encoderPinA, INPUT);
  digitalWrite(encoderPinA, HIGH);         // internal pull-up enabled 
  pinMode(encoderPinB, INPUT);
  digitalWrite(encoderPinB, HIGH);         // internal pull-up enabled  

  // Set up push buttons
  pinMode(Resolution, INPUT);
  digitalWrite(Resolution, HIGH);         // internal pull-up enabled
  pinMode(BandSelect, INPUT);
  digitalWrite(BandSelect, HIGH);         // internal pull-up enabled
  pinMode(FreqDown, INPUT);
  digitalWrite(FreqDown, HIGH);           // internal pull-up enabled
  pinMode(Offset, INPUT);
  digitalWrite(Offset, HIGH);             // internal pull-up enabled
  pinMode(FreqUp, INPUT);
  digitalWrite(FreqUp, HIGH);             // internal pull-up enabled
  pinMode(FreqDown, INPUT);
  digitalWrite(FreqDown, HIGH);           // internal pull-up enabled

  Serial.begin(9600);                     // connect to the port @ 9600 baud

  lcd.display();                          // initialize LCD
  lcd.setCursor(0,1);
  if (GPSflag == 1)
  {
    lcd.print("Waiting for GPS");         // This will continue to display until valid
  }                                       // GPS NMEA data is detected 
  else
  {
    lcd.print("Step:");                   // Frequency step resolution will display only 
    lcd.setCursor(6,1);                   // after valid GPS data is detected or if 
    lcd.print(resolution);                // stand-aloneoperation is selected.
  } 

  TCCR1B = 0;                             //Disable Timer5

  Freq_1 = Freq_array [0] [0];            // At start up load the first band in the the
  Freq_2 = Freq_array [0] [1];            // Freq_array variable.

  setfreq();                              // Now display and set CLK0 and CLK1 frequencies 

}


//******************************************************************
// Loop starts here:
// Loops consecutively to check MCU pins for activity
//******************************************************************
void loop()
{
  if (s_GpsOneSecTick)
  {
    s_GpsOneSecTick = false;
    if (validGPSflag == 1)
    {
      updateAndDisplayUtcTime();

      if ((millis() > time) && (millis() <= time + 1000)) // Test to see if it is time to
      {                                                   // print grid square to the LCD
        lcd.setCursor(0,1);
        lcd.print(GPSlocator);
        lcd.print(" ");
      }
    }

    if (tcount == 1)                                  
    { 
      // Update the SI5351A after every GPS correction 
      si5351aSetFreq(SYNTH_PLL_A, (DIV_50M), Freq_1);
    }
  }
  
  if (GPSflag == 1 && validGPSflag == 0)
  {
    GPSprocess( ); //If GPS is selected, wait for valid NMEA data
  }
  else
  {

    // Rotary encoder algorithm begins here
    byte encoderA = digitalRead(encoderPinA); 
    byte encoderB = digitalRead(encoderPinB);
    if ((encoderA == HIGH) && (encoderC == LOW))
    {
      if (encoderB == LOW)
      {
        // Decrease frequency
        Freq_1 -= fStep; 
      }
      else
      {
        // Increase frequency
        Freq_1 += fStep;
      }
      setfreq();         //Update and display new frequency
      resDisplay();      //Update resolution display and set display timer
    }
    encoderC = encoderA;


    // The frequency step resolution selection begins here:
    if(digitalRead(Resolution) == LOW)
    {
      // This function is used in lieu of "delay" to prevent timer/interrupt conflicts
      for(debounce=0; debounce < DebounceDelay; debounce++){__asm__ __volatile__ ("nop");}; 
      fStepcount++;
      if(fStepcount>6)fStepcount=0;
      setResolution();     // Call the set resolution subroutine
    }

    // Band selection begins here:
    if(digitalRead(BandSelect) == LOW)
    {
      for(debounce=0; debounce < DebounceDelay; debounce++){__asm__ __volatile__ ("nop");};
      band=band+1;                        // Increment band selection
      if(Freq_array [band] [0]==0)band=0; // Check for end of frequency array
      Freq_1 = Freq_array [band] [0];                // Load CLK1 frequency 
      setfreq();                                     // Display and set CLK1 frequency
    }

    // Frequency Up/Down pushbutton algorithm begin here:
    if(digitalRead(FreqUp) == LOW)  // Check for frequency up pushbutton A1 LOW
    {
      for(debounce=0; debounce < DebounceDelay; debounce++){__asm__ __volatile__ ("nop");};
      // Increase frequency by the selected frequency step 
      Freq_1 += fStep;                                // Increase CLK1 by frequency step   
      setfreq();                                      // Set and display new frequency
      resDisplay();                                   // Call the resolution display subroutine
    }

    if(digitalRead(FreqDown) == LOW) // Check for frequency up pushbutton A1 LOW
    {
      for(debounce=0; debounce < DebounceDelay; debounce++){__asm__ __volatile__ ("nop");};
      // Decrease frequency by the selected frequency step and check for 1-80 MHz limits 
      Freq_1 -= fStep;                               // Decrease CLK1 by frequency step 
      setfreq();                                     // Set and display new frequency
      resDisplay();                                  // Call the resolution display subroutine
    }     
  }
}


//******************************************************************
// Add one second to the UTC time and update on the display
//******************************************************************
void updateAndDisplayUtcTime(void)
{
  second++;
  if (second == 60)                            //Set time using GPS NMEA data 
  {
    minute++ ;
    second = 0 ;
  }
  if (minute == 60) 
  {
    hour++;
    minute = 0 ;
  }
  if (hour == 24)
  {
    hour = 0 ;
  }
  //Output the UTC time to the LCD
  lcd.setCursor(8,1);
  if (hour < 10)
  {
    lcd.print ("0");
  }
  lcd.print (hour);
  lcd.print (":");
  if (minute < 10)
  {
    lcd.print ("0");
  }
  lcd.print (minute);
  lcd.print (":");
  if (second < 10)
  {
    lcd.print ("0");
  }
  lcd.print (second);
  lcd.print (" ");  
}

//******************************************************************
// Display and set the Si5351A frequency
//******************************************************************
void setfreq()
{
  unsigned long  Freq_temp = Freq_1; // Temporarily store Freq_1
  char buf[10];

  // Print frequency to the LCD
  lcd.setCursor(0,0);

  ltoa(Freq_temp,buf,10);

  if (Freq_temp < 1000000)
  {
    lcd.print(buf[0]);
    lcd.print(buf[1]);
    lcd.print(buf[2]);
    lcd.print('.');
    lcd.print(buf[3]);
    lcd.print(buf[4]);
    lcd.print(buf[5]);
    lcd.print(" kHz     ");         
  }

  if (Freq_temp >= 1000000 && Freq_temp < 10000000)
  {
    lcd.print(buf[0]);
    lcd.print(',');
    lcd.print(buf[1]);
    lcd.print(buf[2]);
    lcd.print(buf[3]);
    lcd.print('.');
    lcd.print(buf[4]);
    lcd.print(buf[5]);
    lcd.print(buf[6]);
    lcd.print(" kHz   ");
  }

  if (Freq_temp >= 10000000 && Freq_temp < 100000000)
  {
    lcd.print(buf[0]);
    lcd.print(buf[1]);
    lcd.print(',');
    lcd.print(buf[2]);
    lcd.print(buf[3]);
    lcd.print(buf[4]);
    lcd.print('.');
    lcd.print(buf[5]);
    lcd.print(buf[6]);
    lcd.print(buf[7]);
    lcd.print(" kHz  ");
  }

  if (Freq_temp >= 100000000)
  {
    lcd.print(buf[0]);
    lcd.print(buf[1]);
    lcd.print(buf[2]);
    lcd.print(',');
    lcd.print(buf[3]);
    lcd.print(buf[4]);
    lcd.print(buf[5]);
    lcd.print('.');
    lcd.print(buf[6]);
    lcd.print(buf[7]);
    lcd.print(buf[8]);
    lcd.print(" kHz ");
  }  
  si5351aSetFreq(SYNTH_PLL_A, (DIV_50M), Freq_temp); // Set CLK0 frequency
}


//******************************************************************
// Set the frequency step resolution
//******************************************************************
void setResolution()
{
  switch(fStepcount)
  {
  case 0:
    fStep=1;
    resolution = "1 Hz  ";
    break;
  case 1:
    fStep=10;
    resolution = "10 Hz  ";
    break;
  case 2:
    fStep=100;
    resolution = "100 Hz ";
    break;
  case 3:
    fStep=1000;
    resolution = "1 kHz  ";
    break;
  case 4:
    fStep=10000;
    resolution = "10 kHz ";
    break;
  case 5:
    fStep=100000;
    resolution = "100 kHz";
    break;
  case 6:
    fStep=1000000;
    resolution = "1 MHz  ";
    break;
  }
  resDisplay();
}

//******************************************************************
// Display the frequency step resolution and 
// set a 10 second display timer
//******************************************************************
void resDisplay()
{
  if (GPSflag == 0) lcd.setCursor(6,1);
  else lcd.setCursor(0,1);
  lcd.print(resolution);
  time = millis()+10000;
}


//******************************************************************
// Calculate the 6 digit Maidenhead Grid Square location
//******************************************************************
void calcGridSquare()
{
  unsigned long latitude, longitude;
  float temp3,tempLat,tempLong;
  longitude = Lon100*100000000L+Lon10*10000000L+Lon1*1000000L+dLon1*100000L+dLon2*10000L;
  latitude = Lat10*10000000L+Lat1*1000000L+dLat1*100000L+dLat2*10000L;
  tempLong=longitude;
  tempLong=1000000*int(tempLong/1000000)+ ((tempLong-1000000*int(tempLong/1000000))/0.6); 
  if (EW == 'E')tempLong=(tempLong)+180000000;
  if (EW == 'W')tempLong=180000000-(tempLong);
  tempLat=latitude;
  tempLat=1000000*int(tempLat/1000000)+((tempLat-1000000*int(tempLat/1000000))/0.6);   
  if (NS=='N')tempLat=tempLat+90000000;
  if (NS=='S')tempLat=90000000-tempLat;
  GPSlocator[0]=(65+int(tempLong/20000000));
  GPSlocator[1]=(65+int(tempLat/10000000));
  temp3=tempLong-(20000000*int(tempLong/20000000)); 
  GPSlocator[2]=(48+int(temp3*10/20/1000000)); 
  temp3=tempLat-(10000000*int(tempLat/10000000));  
  GPSlocator[3]=(48+int(temp3/1000000)); 
  temp3=(tempLong/2000000)-(int(tempLong/2000000));
  GPSlocator[4]=(97+int(temp3*24));
  temp3=(tempLat/1000000)-(int(tempLat/1000000));
  GPSlocator[5]=(97+int(temp3*24));
}


//******************************************************************
//  GPS NMEA processing starts here
//******************************************************************
void GPSprocess()
{
  byte temp, pinState = 0, i;

  if (Serial.available() > 0)
  {
    // NMEA $GPGGA data begins here
    byteGPS = Serial.read();     // Read a byte of the serial port
    buffer[counter] = byteGPS;   // If there is serial port data, it is put in the buffer
    Serial.write(byteGPS);
    counter++;                      
    if (byteGPS == 13){            // If the received byte is = to 13, end of transmission
      IndiceCount = 0;
      StartCount = 0;
      for (int i=1;i<7;i++){     // Verifies if the received command starts with $GPGGA
        if (buffer[i]==StartCommand[i-1]){
          StartCount++;
        }
      }
      if(StartCount==6){         // If yes, continue and process the data
        for (int i=0;i<300;i++){
          if (buffer[i]==','){   // check for the position of the  "," separator
            indices[IndiceCount]=i;
            IndiceCount++;
          }
          if (buffer[i]=='*'){    // ... and the "*"
            indices[12]=i;
            IndiceCount++;
          }
        }
        // Load the NMEA time data
        temp = indices[0];
        hour = (buffer[temp+1]-48)*10 + buffer[temp+2]-48;
        minute = (buffer[temp+3]-48)*10 + buffer[temp+4]-48;
        second = (buffer[temp+5]-48)*10 + buffer[temp+6]-48; 
        // Load latitude and logitude data          
        temp = indices[1];
        Lat10 = buffer[temp+1]-48;
        Lat1 = buffer[temp+2]-48;
        dLat1 = buffer[temp+3]-48;
        dLat2 = buffer[temp+4]-48;
        temp = indices[2];
        NS = buffer[temp+1];
        temp = indices[3];
        Lon100 = buffer[temp+1]-48;
        Lon10 = buffer[temp+2]-48;
        Lon1 = buffer[temp+3]-48; 
        dLon1 = buffer[temp+4]-48;
        dLon2 = buffer[temp+5]-48;        
        temp = indices[4];
        EW = buffer[temp+1];
        temp = indices[5];
        validGPSflag = buffer[temp+1]-48;     
      }

      else
      { // NMEA $GPRMC data begins here
        // Note: &GPRMC does not include satellite in view data
        IndiceCount=0;
        StartCount=0;
        for (int i=1;i<7;i++){     // Verifies if the received command starts with $GPRMC
          if (buffer[i]==StartCommand2[i-1]){
            StartCount++;
          }
        }       
        if(StartCount==6){         // If yes, continue and process the data
          for (int i=0;i<300;i++){
            if (buffer[i]==','){   // check for the position of the  "," separator
              indices[IndiceCount]=i;
              IndiceCount++;
            }
            if (buffer[i]=='*'){   // ... and the "*"
              indices[12]=i;
              IndiceCount++;
            }
          }
          // Load time data
          temp = indices[0];
          hour = (buffer[temp+1]-48)*10 + buffer[temp+2]-48;
          minute = (buffer[temp+3]-48)*10 + buffer[temp+4]-48;
          second = (buffer[temp+5]-48)*10 + buffer[temp+6]-48;
          temp = indices[1]; 
          if (buffer[temp+1] == 65){
            validGPSflag = 1; 
          }
          else
          {
            validGPSflag = 0;
          }        
          // Load latitude and logitude data          
          temp = indices[2];
          Lat10 = buffer[temp+1]-48;
          Lat1 = buffer[temp+2]-48;
          dLat1 = buffer[temp+3]-48;
          dLat2 = buffer[temp+4]-48;
          temp = indices[3];
          NS = buffer[temp+1];
          temp = indices[4];
          Lon100 = buffer[temp+1]-48;
          Lon10 = buffer[temp+2]-48;
          Lon1 = buffer[temp+3]-48;
          dLon1 = buffer[temp+4]-48;
          dLon2 = buffer[temp+5]-48;         
          temp = indices[5];
          EW = buffer[temp+1];
        }
      }
      if(validGPSflag == 1) 
      {
        calcGridSquare();
        lcd.setCursor(0,1);
        lcd.print(GPSlocator);
        lcd.print("  ");
      }
      counter=0;                  // Reset the buffer
      for (int i=0;i<300;i++){    //  
        buffer[i]=' '; 
      }
    }
  }
}

//******************************************************************
//  Si5351 Multisynth processing
//******************************************************************
void si5351aSetFreq(int synth, int odiv, unsigned long freq)
{
  unsigned long long CalcTemp;
  unsigned long  a, b, c, p1, p2, p3;

  // PLL multiplier = a + b/c 
  //                  = (freq * output_divisor) / (GPS_clock_count / 4) 
  //                  = (freq * 4 * output_divisor) / (GPS_clock_count)
  
  c = 0xFFFFF;  // Denominator derived from max bits 2^20
  CalcTemp = odiv;
  CalcTemp *= (freq * 4);
  a= CalcTemp/XtalFreq;
  CalcTemp %= XtalFreq;
  CalcTemp *= c;
  CalcTemp /= XtalFreq;
  b = CalcTemp;
  
  // Refer to Si5351 Register Map AN619 for following formula
  p3  = c;
  p2  = (128 * b) % c;
  p1  = 128 * a;
  p1 += (128 * b / c);
  p1 -= 512;
  
  // Write data to multisynth registers
  Si5351_write(synth, 0xFF);  
  Si5351_write(synth + 1, 0xFF);
  Si5351_write(synth + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(synth + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(synth + 4, (p1 & 0x000000FF));
  Si5351_write(synth + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(synth + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(synth + 7, (p2 & 0x000000FF));
}


//******************************************************************
//  Si5351 initialization routines
//******************************************************************
void si5351aStart()
{
  
  // Initialize Si5351A
  Si5351_write(XTAL_LOAD_CAP,0b11000000);      // Set crystal load to 10pF
  Si5351_write(CLK_ENABLE_CONTROL,0b00000000); // Enable all outputs
  Si5351_write(CLK0_CONTROL,0b00001111);       // Set PLLA to CLK0, 8 mA output
  Si5351_write(CLK1_CONTROL,0b00001111);       // Set PLLA to CLK1, 8 mA output
  Si5351_write(CLK2_CONTROL,0b00101111);       // Set PLLB to CLK2, 8 mA output
  Si5351_write(PLL_RESET,0b10100000);          // Reset PLLA and PLLB


  // Set PLLB to 800 MHz
  int synth;
  unsigned long  a, b, c, p1, p2, p3;

  a = 32;           // Derived from 800/25 MHz
  b = 0;            // Numerator
  c = 0xFFFFF;      // Denominator derived from max bits 2^20

  // Refer to Si5351 Register Map AN619 for following formula
  p3  = c;
  p2  = (128 * b) % c;
  p1  = 128 * a;
  p1 += (128 * b / c);
  p1 -= 512;

  // Write data to PLL registers

  synth = (SYNTH_PLL_B);
  
  Si5351_write(synth, 0xFF);  
  Si5351_write(synth + 1, 0xFF);
  Si5351_write(synth + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(synth + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(synth + 4, (p1 & 0x000000FF));
  Si5351_write(synth + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(synth + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(synth + 7, (p2 & 0x000000FF));

  // initialize MultiSynth output dividers:

  // CLK2 is 800MHZ/320 = 2.5MHz
  a = 320;
  b = 0;
  
  // Refer to Si5351 Register Map AN619 for following formula
  p3  = c;
  p2  = (128 * b) % c;
  p1  = 128 * a;
  p1 += (128 * b / c);
  p1 -= 512;

  synth = (SYNTH_MS_2);
  Si5351_write(synth, 0xFF);  
  Si5351_write(synth + 1, 0xFF);
  Si5351_write(synth + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(synth + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(synth + 4, (p1 & 0x000000FF));
  Si5351_write(synth + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(synth + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(synth + 7, (p2 & 0x000000FF));


  // CLK0 and CLK1 are the same frequency, in quadrature 
  a = (DIV_50M);
  b = 0;
  
  // Refer to Si5351 Register Map AN619 for following formula
  p3  = c;
  p2  = (128 * b) % c;
  p1  = 128 * a;
  p1 += (128 * b / c);
  p1 -= 512;

  synth = (SYNTH_MS_0);
  Si5351_write(synth, 0xFF);  
  Si5351_write(synth + 1, 0xFF);
  Si5351_write(synth + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(synth + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(synth + 4, (p1 & 0x000000FF));
  Si5351_write(synth + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(synth + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(synth + 7, (p2 & 0x000000FF));

  
  synth = (SYNTH_MS_1);
  Si5351_write(synth, 0xFF);  
  Si5351_write(synth + 1, 0xFF);
  Si5351_write(synth + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(synth + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(synth + 4, (p1 & 0x000000FF));
  Si5351_write(synth + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(synth + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(synth + 7, (p2 & 0x000000FF));

  Si5351_write(CLK1_PHOFF, a);
  


  Si5351_write(PLL_RESET,0b10100000);          // Reset PLLA and PLLB

}

//******************************************************************
//Write I2C data routine
//******************************************************************
uint8_t Si5351_write(uint8_t addr, uint8_t data)
{
  Wire.beginTransmission(Si5351A_addr);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
}
//------------------------------------------------------------------------------------------------------
