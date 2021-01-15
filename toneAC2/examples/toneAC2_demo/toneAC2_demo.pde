// ---------------------------------------------------------------------------
// Be sure to include an in-line 100 ohm resistor on one pin as you normally do when connecting a piezo or speaker.
// ---------------------------------------------------------------------------

#include <toneAC2.h>

// Melody liberated from the toneMelody Arduino example sketch by Tom Igoe.
int melody[] = { 262, 196, 196, 220, 196, 0, 247, 262 };
int noteDurations[] = { 4, 8, 8, 4, 4, 4, 4, 4 };

void setup() {} // Nothing to setup, just start playing!

void loop() {
  for (unsigned long freq = 125; freq <= 15000; freq += 10) {  
    toneAC2(2, 3, freq, 1); // Play the frequency (125 Hz to 15 kHz sweep in 10 Hz steps) for 1ms.
  }

  delay(1000); // Wait a second.

  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000/noteDurations[thisNote];
    toneAC2(2, 3, melody[thisNote], noteDuration, true); // Play thisNote at full volume for noteDuration in the background.
    delay(noteDuration * 4 / 3); // Wait while the tone plays in the background, plus another 33% delay between notes.
  }

  while(1); // Stop (so it doesn't repeat forever driving you crazy--you're welcome).
}
