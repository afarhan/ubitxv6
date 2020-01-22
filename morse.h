#include "settings.h"
//sends out morse code at the speed set by cwSpeed
void morseLetter(char c, uint16_t dit_duration_ms = globalSettings.cwDitDurationMs);
void morseText(char *text, uint16_t dit_duration_ms = globalSettings.cwDitDurationMs);
