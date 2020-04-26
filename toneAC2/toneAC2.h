// ---------------------------------------------------------------------------
// toneAC2 Library - v1.1 - 09/15/2015
//
// AUTHOR/LICENSE:
// Created by Tim Eckel - teckel@leethost.com
// Copyright 2015 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html
//
// LINKS:
// Project home: https://bitbucket.org/teckel12/arduino-toneac/wiki/Home
// Blog: http://forum.arduino.cc/index.php?topic=142097.0
//
// DISCLAIMER:
// This software is furnished "as is", without technical support, and with no 
// warranty, express or implied, as to its usefulness for any purpose.
//
// PURPOSE:
// Replacement to the standard tone library with the advantage of nearly twice
// the volume, 800 bytes smaller compiled code size, and less stress on the
// speaker. This alternate version uses timer 2 and allows for flexible pin
// assignment. The primary version (toneAC) allows for higher frequencies,
// higher quality, and even smaller code size. However, toneAC is fixed to
// using the PWM timer 1 pins unlike toneAC2 which can use any two pins. Both
// exclusively use port registers for the fast and smallest code possible.
//
// USAGE:
// Connection is very similar to a piezo or standard speaker. Except, instead
// of connecting one speaker wire to ground you connect both speaker wires to
// Arduino pins. Unlike toneAC, with toneAC2 you can connect to any two pins.
// Just as usual when connecting a speaker, make sure you add an in-line 100
// ohm resistor between one of the pins and the speaker wire.
//
// SYNTAX:
//   toneAC2( pin1, pin2, frequency [, length [, background ]] ) - Play a note.
//     Parameters:
//       * pin1       - Pin to attach one of the speaker wires.
//       * pin2       - Pin to attach the other speaker wire.
//       * frequency  - Play the specified frequency indefinitely, turn off with noToneAC2().
//       * length     - [optional] Set the length to play in milliseconds. (default: 0 [forever], range: 0 to 2^32-1)
//       * background - [optional] Play note in background or pause till finished? (default: false, values: true/false)
//   noToneAC2()  - Stop playing.
//
// HISTORY:
// 09/15/2015 v1.1 - Fix a potential race condition with _tAC2_time. Moved
// development to Bitbucket.
//
// 01/27/2013 v1.0 - Initial release.
//
// ---------------------------------------------------------------------------

#ifndef toneAC2_h
  #define toneAC2_h

  #if defined(ARDUINO) && ARDUINO >= 100
    #include <Arduino.h>
  #else
    #include <WProgram.h>
  #endif

  // This doesn't currently work. Would require more work than simply doing this.
  #if defined(__AVR_ATmega8__) || defined(__AVR_ATmega128__)
    #define TCCR2A TCCR2
    #define TCCR2B TCCR2
    #define TIMSK2 TIMSK
    #define COM2A1 COM21
    #define COM2A0 COM20
    #define OCIE2A OCIE2
    #define OCR2A OCR2
    #define TIMER2_COMPA_vect TIMER2_COMP_vect
  #endif

  void toneAC2(uint8_t pin1, uint8_t pin2, unsigned int frequency = 0, unsigned long length = 0, uint8_t background = false);
  void noToneAC2();
#endif