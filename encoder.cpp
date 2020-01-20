#include <Arduino.h>
#include <stdint.h>

#include "ubitx.h"//Pin definitions


/*
 * SmittyHalibut's encoder handling, using interrupts. Should be quicker, smoother handling.
 */
int8_t enc_count;
uint8_t prev_enc;

uint8_t enc_state (void) {
    return (digitalRead(ENC_A)?1:0 + digitalRead(ENC_B)?2:0);
}

/*
 * The Interrupt Service Routine for Pin Change Interrupts on A0-A5.
 */
ISR (PCINT1_vect) {
  uint8_t cur_enc = enc_state();
  if (prev_enc == cur_enc) {
    //Serial.println("unnecessary ISR");
    return;
  }
  //Serial.print(prev_enc);
  //Serial.println(cur_enc);
  
  //these transitions point to the enccoder being rotated anti-clockwise
  if ((prev_enc == 0 && cur_enc == 2) || 
      (prev_enc == 2 && cur_enc == 3) || 
      (prev_enc == 3 && cur_enc == 1) || 
      (prev_enc == 1 && cur_enc == 0))
  {
    enc_count-=1;
  }
  //these transitions point to the enccoder being rotated clockwise
  else if ((prev_enc == 0 && cur_enc == 1) || 
      (prev_enc == 1 && cur_enc == 3) || 
      (prev_enc == 3 && cur_enc == 2) || 
      (prev_enc == 2 && cur_enc == 0))
  {
    enc_count+=1;
  }
  else {
    // A change to two states, we can't tell whether it was forward or backward, so we skip it.
    //Serial.println("skip");
  }
  prev_enc = cur_enc; // Record state for next pulse interpretation
}

/*
 * Setup the encoder interrupts and global variables.
 */
void pci_setup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void enc_setup(void)
{
  enc_count = 0;
  // This is already done in setup() ?
  //pinMode(ENC_A, INPUT);
  //pinMode(ENC_B, INPUT);
  prev_enc = enc_state();

  // Setup Pin Change Interrupts for the encoder inputs
  pci_setup(ENC_A);
  pci_setup(ENC_B);
}

int enc_read(void) {
  int8_t ret = enc_count;
  enc_count = 0;
  return int(ret);
}