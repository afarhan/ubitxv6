/*
 * Formats the frequency given
 */
void formatFreq(uint32_t freq, char* buff, uint16_t buff_size, uint8_t fixed_width)
{
  memset(buff, 0, buff_size);

  ultoa(freq, buff, DEC);
  uint8_t num_digits = strlen(buff);
  const uint8_t num_spacers = (num_digits-1) / 3;
  const uint8_t num_leading_digits_raw = num_digits % 3;
  const uint8_t num_leading_digits = (0 == num_leading_digits_raw) ? 3 : num_leading_digits_raw;

  if(0 < fixed_width){
    while(0 < fixed_width - num_digits - num_spacers){
      if(0 == fixed_width % 4){
        buff[0] = '\x81';//separator size
      }
      else{
        buff[0] = '\x80';//digit size
      }
      --fixed_width;
      ++buff;
    }
  }

  ultoa(freq, buff, DEC);
  buff += num_leading_digits;
  num_digits -= num_leading_digits;
  for(int i = num_digits-1; i >= 0; --i){
    buff[i + (i/3 + 1)] = buff[i];
  }
  for(unsigned int i = 0; i < num_spacers; ++i){
    memcpy_P(buff,F("."),1);
    buff += 4;
  }
}
