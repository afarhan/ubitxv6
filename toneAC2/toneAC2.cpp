// ---------------------------------------------------------------------------
// Created by Tim Eckel - teckel@leethost.com
// Copyright 2015 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html
//
// See "toneAC2.h" for purpose, syntax, version history, links, and more.
// ---------------------------------------------------------------------------

#include "toneAC2.h"

unsigned long _tAC2_time; // Used to track end note with timer when playing note in the background.
volatile uint8_t *_pinMode1, *_pinMode2;     // Pin modes.
uint8_t _pinMask1 = 0, _pinMask2 = 0;        // Bitmask for pins.
volatile uint8_t *_pinOutput1, *_pinOutput2; // Output port registers for each pin.
int _tAC2_prescale[] = { 2, 16, 64, 128, 256, 512, 2048 }; // Prescaler.

void toneAC2(uint8_t pin1, uint8_t pin2, unsigned int frequency, unsigned long length, uint8_t background) {
  long top;
  uint8_t prescaler;

  for (prescaler = 1; prescaler < 8; prescaler++) { // Find the appropriate prescaler
    top = F_CPU / (long) frequency / (long) _tAC2_prescale[prescaler - 1] - 1; // Calculate the top.
    if (top < 256) break; // Fits, break out of for loop.
  }
  if (top > 255) { noToneAC2(); return; } // Frequency is out of range, turn off sound and return.

  if (length > 0) _tAC2_time = millis() + length - 1; else _tAC2_time = 0xFFFFFFFF; // Set when the note should end, or play "forever".

  if (_pinMask1 == 0) { // This gets the port registers and bitmaps for the two pins and sets the pins to output mode.
    _pinMask1   = digitalPinToBitMask(pin1);                            // Get the port register bitmask for pin 1.
    _pinMask2   = digitalPinToBitMask(pin2);                            // Get the port register bitmask for pin 2.
    _pinOutput1 = portOutputRegister(digitalPinToPort(pin1));           // Get the output port register for pin 1.
    _pinOutput2 = portOutputRegister(digitalPinToPort(pin2));           // Get the output port register for pin 2.
    _pinMode1   = (uint8_t *) portModeRegister(digitalPinToPort(pin1)); // Get the port mode register for pin 1.
    _pinMode2   = (uint8_t *) portModeRegister(digitalPinToPort(pin2)); // Get the port mode register for pin 2.
    *_pinMode1 |= _pinMask1; // Set pin 1 to Output mode.
    *_pinMode2 |= _pinMask2; // Set pin 2 to Output mode.
  }

  OCR2A   = top;                     // Set the top.
  if (TCNT2 > top) TCNT2 = top;      // Counter over the top, put within range.
  TCCR2B  = _BV(WGM22) | prescaler;  // Set Fast PWM and prescaler.
  TCCR2A  = _BV(WGM20) | _BV(WGM21); // Fast PWM and normal port operation, OC2A/OC2B disconnected.
  TIMSK2 &= ~_BV(OCIE2A);            // Stop timer 2 interrupt while we set the pin states.
  if (*_pinOutput1 & _pinMask1) *_pinOutput2 &= ~_pinMask2; // Be sure pins are reversed.
  else *_pinOutput2 |= _pinMask2;    // Other half of making sure pins are reversed.
  TIMSK2 |= _BV(OCIE2A);             // Activate the timer interrupt.
  
  if (length > 0 && !background) { delay(length); noToneAC2(); } // Just a simple delay, doesn't return control till finished.
}

void noToneAC2() {
  TIMSK2 &= ~_BV(OCIE2A);     // Remove the timer interrupt.
  TCCR2B  = _BV(CS22);        // Default clock prescaler of 64.
  TCCR2A  = _BV(WGM20);       // Set to defaults so PWM can work like normal (PWM, phase corrected, 8bit).
  *_pinMode1 &= ~_pinMask1;   // Set pin 1 to INPUT.
  *_pinMode2 &= ~_pinMask2;   // Set pin 2 to INPUT.
  _pinMask1 = 0; // Flag so we know note is no longer playing.
}

ISR(TIMER2_COMPA_vect) { // Timer interrupt vector.
  if (millis() > _tAC2_time) noToneAC2(); // Check to see if it's time for the note to end.
  *_pinOutput1 ^= _pinMask1; // Toggle the pin 1 state.
  *_pinOutput2 ^= _pinMask2; // Toggle the pin 2 state.
}