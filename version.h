#pragma once

#include <avr/pgmspace.h>

/*
 * VERSION_STRING is a PROGMEM string, so extract it before use, e.g.
 *      strncpy_P(char_buffer_out,VERSION_STRING,size_of_char_buffer_out);
 */
extern const char* const VERSION_STRING;