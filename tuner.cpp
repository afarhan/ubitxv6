#include "tuner.h"

#include <Arduino.h>

#include "nano_gui.h"
#include "pin_definitions.h"
#include "si5351.h"

static const uint32_t THRESHOLD_USB_LSB = 10000000L;

void saveVFOs()
{
  SaveSettingsToEeprom();
}


void switchVFO(Vfo_e new_vfo){
  ritDisable();//If we are in RIT mode, we need to disable it before setting the active VFO so that the correct VFO gets it's frequency restored

  globalSettings.activeVfo = new_vfo;
  setFrequency(GetActiveVfoFreq());
  saveVFOs();
}

/**
 * Select the properly tx harmonic filters
 * The four harmonic filters use only three relays
 * the four LPFs cover 30-21 Mhz, 18 - 14 Mhz, 7-10 MHz and 3.5 to 5 Mhz
 * Briefly, it works like this, 
 * - When KT1 is OFF, the 'off' position routes the PA output through the 30 MHz LPF
 * - When KT1 is ON, it routes the PA output to KT2. Which is why you will see that
 *   the KT1 is on for the three other cases.
 * - When the KT1 is ON and KT2 is off, the off position of KT2 routes the PA output
 *   to 18 MHz LPF (That also works for 14 Mhz) 
 * - When KT1 is On, KT2 is On, it routes the PA output to KT3
 * - KT3, when switched on selects the 7-10 Mhz filter
 * - KT3 when switched off selects the 3.5-5 Mhz filter
 * See the circuit to understand this
 */

void setTXFilters(unsigned long freq){
  
  if (freq > 21000000L){  // the default filter is with 35 MHz cut-off
    digitalWrite(PIN_TX_LPF_A, 0);
    digitalWrite(PIN_TX_LPF_B, 0);
    digitalWrite(PIN_TX_LPF_C, 0);
  }
  else if (freq >= 14000000L){ //thrown the KT1 relay on, the 30 MHz LPF is bypassed and the 14-18 MHz LPF is allowd to go through
    digitalWrite(PIN_TX_LPF_A, 1);
    digitalWrite(PIN_TX_LPF_B, 0);
    digitalWrite(PIN_TX_LPF_C, 0);
  }
  else if (freq > 7000000L){
    digitalWrite(PIN_TX_LPF_A, 0);
    digitalWrite(PIN_TX_LPF_B, 1);
    digitalWrite(PIN_TX_LPF_C, 0);
  }
  else {
    digitalWrite(PIN_TX_LPF_A, 0);
    digitalWrite(PIN_TX_LPF_B, 0);
    digitalWrite(PIN_TX_LPF_C, 1);
  }
}

/**
 * This is the most frequently called function that configures the 
 * radio to a particular frequeny, sideband and sets up the transmit filters
 * 
 * The transmit filter relays are powered up only during the tx so they dont
 * draw any current during rx. 
 * 
 * The carrier oscillator of the detector/modulator is permanently fixed at
 * uppper sideband. The sideband selection is done by placing the second oscillator
 * either 12 Mhz below or above the 45 Mhz signal thereby inverting the sidebands 
 * through mixing of the second local oscillator.
 */
 
void setFrequency(const unsigned long freq,
                  const bool transmit){
  static const unsigned long FIRST_IF = 45005000UL;
 
  setTXFilters(freq);

  //Nominal values for the oscillators
  uint32_t local_osc_freq = FIRST_IF + freq;
  uint32_t ssb_osc_freq = FIRST_IF;//will be changed depending on sideband
  uint32_t bfo_osc_freq = globalSettings.usbCarrierFreq;

  if(TuningMode_e::TUNE_CW == globalSettings.tuningMode){
    if(transmit){
      //We don't do any mixing or converting when transmitting
      local_osc_freq = freq;
      ssb_osc_freq = 0;
      bfo_osc_freq = 0;
    }
    else{
      //We offset when receiving CW so that it's audible
      if(VfoMode_e::VFO_MODE_USB == GetActiveVfoMode()){
        local_osc_freq -= globalSettings.cwSideToneFreq;
        ssb_osc_freq += globalSettings.usbCarrierFreq;
      }
      else{
        local_osc_freq += globalSettings.cwSideToneFreq;
        ssb_osc_freq -= globalSettings.usbCarrierFreq;
      }
    }
  }
  else{//SSB mode
    if(VfoMode_e::VFO_MODE_USB == GetActiveVfoMode()){
      ssb_osc_freq += globalSettings.usbCarrierFreq;
    }
    else{
      ssb_osc_freq -= globalSettings.usbCarrierFreq;
    }
  }

  si5351bx_setfreq(2, local_osc_freq);
  si5351bx_setfreq(1, ssb_osc_freq);
  si5351bx_setfreq(0, bfo_osc_freq);

  SetActiveVfoFreq(freq);
}

/**
 * startTx is called by the PTT, cw keyer and CAT protocol to
 * put the uBitx in tx mode. It takes care of rit settings, sideband settings
 * Note: In cw mode, doesnt key the radio, only puts it in tx mode
 * CW offest is calculated as lower than the operating frequency when in LSB mode, and vice versa in USB mode
 */
 
void startTx(TuningMode_e tx_mode){
  globalSettings.tuningMode = tx_mode;

  if (globalSettings.ritOn){
    //save the current as the rx frequency
    uint32_t rit_tx_freq = globalSettings.ritFrequency;
    globalSettings.ritFrequency = GetActiveVfoFreq();
    setFrequency(rit_tx_freq,true);
  }
  else{
    if(globalSettings.splitOn){
      if(Vfo_e::VFO_B == globalSettings.activeVfo){
        globalSettings.activeVfo = Vfo_e::VFO_A;
      }
      else{
        globalSettings.activeVfo = Vfo_e::VFO_B;
      }
    }
    setFrequency(GetActiveVfoFreq(),true);
  }

  digitalWrite(PIN_TX_RXn, 1);//turn on the tx
  globalSettings.txActive = true;
  drawTx();
}

void stopTx(){
  digitalWrite(PIN_TX_RXn, 0);//turn off the tx
  globalSettings.txActive = false;

  if(globalSettings.ritOn){
    uint32_t rit_rx_freq = globalSettings.ritFrequency;
    globalSettings.ritFrequency = GetActiveVfoFreq();
    setFrequency(rit_rx_freq);
  }
  else{
    if(globalSettings.splitOn){
      if(Vfo_e::VFO_B == globalSettings.activeVfo){
        globalSettings.activeVfo = Vfo_e::VFO_A;
      }
      else{
        globalSettings.activeVfo = Vfo_e::VFO_B;
      }
    }
    setFrequency(GetActiveVfoFreq());
  }
  drawTx();
}

/**
 * ritEnable is called with a frequency parameter that determines
 * what the tx frequency will be
 */
void ritEnable(unsigned long freq){
  globalSettings.ritOn = true;
  //save the non-rit frequency back into the VFO memory
  //as RIT is a temporary shift, this is not saved to EEPROM
  globalSettings.ritFrequency = freq;
}

// this is called by the RIT menu routine
void ritDisable(){
  if(globalSettings.ritOn){
    globalSettings.ritOn = false;
    setFrequency(globalSettings.ritFrequency);
  }
}

bool autoSelectSidebandChanged(const uint32_t old_frequency)
{
  const uint32_t new_freq = GetActiveVfoFreq();
  //Transition from below to above the traditional threshold for USB
  if(old_frequency < THRESHOLD_USB_LSB && new_freq >= THRESHOLD_USB_LSB){
    SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
    setFrequency(new_freq);//Refresh tuning to activate the new sideband mode
    return true;
  }
  
  //Transition from above to below the traditional threshold for USB
  if(old_frequency >= THRESHOLD_USB_LSB && new_freq < THRESHOLD_USB_LSB){
    SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
    setFrequency(new_freq);//Refresh tuning to activate the new sideband mode
    return true;
  }

  return false;
}
