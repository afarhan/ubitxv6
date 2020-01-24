#include <Arduino.h>
#include <stdint.h>
#include "ubitx.h"//Pin definitions

//Normal encoder state
uint8_t prev_enc = 0;
int8_t enc_count = 0;

//Momentum encoder state
int16_t enc_count_periodic = 0;
int8_t momentum[3] = {0};
static const uint16_t CALLBACK_PERIOD_MS = 200;
static const uint8_t MOMENTUM_MULTIPLIER = 1;

uint8_t enc_state (void)
{
  return (digitalRead(ENC_A)?1:0 + digitalRead(ENC_B)?2:0);
}

/*
 * SmittyHalibut's encoder handling, using interrupts. Should be quicker, smoother handling.
 * The Interrupt Service Routine for Pin Change Interrupts on A0-A5.
 */
ISR (PCINT1_vect)
{
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
    enc_count -= 1;
    enc_count_periodic -= 1;
  }
  //these transitions point to the enccoder being rotated clockwise
  else if ((prev_enc == 0 && cur_enc == 1) || 
      (prev_enc == 1 && cur_enc == 3) || 
      (prev_enc == 3 && cur_enc == 2) || 
      (prev_enc == 2 && cur_enc == 0))
  {
    enc_count += 1;
    enc_count_periodic += 1;
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

  //Set up timer interrupt for momentum
  TCCR1A = 0;//"normal" mode
  TCCR1B = 3;//clock divider of 64
  TCNT1  = 0;//start counting at 0
  OCR1A  = F_CPU * CALLBACK_PERIOD_MS / 1000 / 64;//set target number
  TIMSK1 |= (1 << OCIE1A);//enable interrupt
}

ISR(TIMER1_COMPA_vect)
{
  momentum[2] = momentum[1];
  momentum[1] = momentum[0];
  momentum[0] = enc_count_periodic;
  enc_count_periodic = 0;
}

int8_t min_momentum_mag()
{
  int8_t min_mag = 127;
  for(uint8_t i = 0; i < sizeof(momentum)/sizeof(momentum[0]); ++i){
    int8_t mag = abs(momentum[i]);
    if(mag < min_mag){
      min_mag = mag;
    }
  }
  return min_mag;
}

int enc_read(void) {
  if(0 != enc_count){
    int16_t ret = enc_count;
    int8_t s = (enc_count < 0) ? -1 : 1;
    int8_t momentum_mag = min_momentum_mag();
    if(momentum_mag >= 20){
      ret += s*40;
    }
    else if(momentum_mag >= 5){
      ret += s*(20 + momentum_mag)/(20 - momentum_mag);
    }
    enc_count = 0;
    return ret;
  }
  return 0;
}
