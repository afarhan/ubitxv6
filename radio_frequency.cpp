#include "radio.h"
#include "ubitx_si5351.h"
#include "loop_master.h"

void radio::set_cw_oscillators(void) {
  //turn off the second local oscillator and the bfo
  long f = vfo_data[split ^ using_vfo_b].frequency;
  #ifdef USE_ATTN
  set_attn();
  #endif
  if (cw_tx) {
    setTXFilters(f);
    si5351bx_setfreq(0, 0);
    si5351bx_setfreq(1, 0);
    si5351bx_setfreq(2, f);
    #ifndef USE_RX_ONLY
      #ifdef USE_TX_DIS
      digitalWrite(TX_RX, tx_enable);  
      #else
      digitalWrite(TX_RX, true);
      #endif
    #endif
  } else {
    digitalWrite(TX_RX, 0);
    digitalWrite(CW_KEY, 0); 
    // si5351_set_calibration(calibration);  // restore clock 0
    si5351bx_setfreq(0, radio_obj.usbCarrier);
    setHWFrequencyRX();
  }
  update_both_vfos();
}

#ifdef USE_TUNE
void radio::toggle_tune(void) {
  tune = !tune;
  in_tx = tune;
  sidetone(tune);
  #ifdef USE_ATTN
  set_attn();
  #endif
  if (tune) {
    long f = vfo_data[split ^ using_vfo_b].frequency;
    setTXFilters(f);
    si5351bx_setfreq(0, 0);
    si5351bx_setfreq(1, 0);
    si5351bx_setfreq(2, f);
    #ifndef USE_RX_ONLY
      #ifdef USE_TX_DIS
      digitalWrite(TX_RX, tx_enable); 
      #else
      digitalWrite(TX_RX, true); 
      #endif
    #endif
  } else {
    digitalWrite(TX_RX, 0);
    digitalWrite(CW_KEY, 0); 
    //si5351_set_calibration(calibration);
    si5351bx_setfreq(0, radio_obj.usbCarrier);
    setHWFrequencyRX();
  }
  update_both_vfos();
  loop_master::active->update_tune();
}
#endif

void radio::setHWFrequencyTX(void) {
  unsigned long f = vfo_data[split ^ using_vfo_b].frequency;
  setTXFilters(f);
  si5351bx_setfreq(2, firstIF + f);                              // 1st IF
  si5351bx_setfreq(1, firstIF + (vfo_data[using_vfo_b].usb ? usbCarrier : -usbCarrier));  // 2nd IF
}

void radio::setHWFrequencyRX(void) {
  #if defined USE_RF_SHIFT || defined USE_RF_SHIFT_STEP
    si5351bx_setfreq(2, rf_shift_freq + adjusted_frequency(using_vfo_b) + ((!in_tx && vfo_data[using_vfo_b].cw) ? (vfo_data[using_vfo_b].usb ? -sidetone_frequency : sidetone_frequency) : 0)); // 1st IF
    #ifdef USE_IF_SHIFT
      si5351bx_setfreq(1, rf_shift_freq + (vfo_data[using_vfo_b].usb ? if_shift_freq : -if_shift_freq));  // 2nd IF
    #else
      si5351bx_setfreq(1, rf_shift_freq + (vfo_data[using_vfo_b].usb ? usbCarrier : -usbCarrier));  // 2nd IF
    #endif
  #else
    si5351bx_setfreq(2, firstIF + adjusted_frequency(using_vfo_b) + ((!in_tx && vfo_data[using_vfo_b].cw) ? (vfo_data[using_vfo_b].usb ? -sidetone_frequency : sidetone_frequency) : 0)); // 1st IF
    #ifdef USE_IF_SHIFT
      si5351bx_setfreq(1, firstIF + (vfo_data[using_vfo_b].usb ? if_shift_freq : -if_shift_freq));  // 2nd IF
    #else
      si5351bx_setfreq(1, firstIF + (vfo_data[using_vfo_b].usb ? usbCarrier : -usbCarrier));  // 2nd IF
    #endif
  #endif
}

#if 0
void radio::setHWFrequencyRX(void) {
  unsigned long f = adjusted_frequency(using_vfo_b);
   setTXFilters(f);
  si5351bx_setfreq(2, firstIF + f + ((!in_tx && vfo_data[using_vfo_b].cw) ? (vfo_data[using_vfo_b].usb ? -sidetone_frequency : sidetone_frequency) : 0)); // 1st IF
  #ifdef USE_IF_SHIFT
  si5351bx_setfreq(1, firstIF + (vfo_data[using_vfo_b].usb ? (usbCarrier + if_shift) : -(usbCarrier + if_shift)));  // 2nd IF
  #else
  si5351bx_setfreq(1, firstIF + (vfo_data[using_vfo_b].usb ? usbCarrier : -usbCarrier));  // 2nd IF
  #endif
}
#endif

