#include <stdint.h>

void getBandString(const uint32_t frequency,
                   char* band_string_out,
                   const uint16_t max_string_length);

uint32_t getFreqInBand(const uint32_t frequency,
                       const uint8_t target_band);

bool isFreqInBand(const uint32_t frequency,
                  const uint8_t band);