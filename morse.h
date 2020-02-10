#include "settings.h"
//sends out morse code at the speed set by cwSpeed
void morseLetter(char c, uint16_t dit_duration_ms = globalSettings.cwDitDurationMs);
void morseText(const char *text, uint16_t dit_duration_ms = globalSettings.cwDitDurationMs);

//Plays either a higher or lower tone to indicate a boolean value
void morseBool(bool val);
