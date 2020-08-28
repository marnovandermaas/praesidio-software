#include "praesidiooutput.h"

void output_hexbyte(unsigned char c) {
  unsigned char upper = (c >> 4);
  unsigned char lower = (c & 0xF);
  if(upper < 10) {
    OUTPUT_CHAR(upper + '0');
  } else {
    OUTPUT_CHAR(upper + 'A' - 10);
  }
  if(lower < 10) {
    OUTPUT_CHAR(lower + '0');
  } else {
    OUTPUT_CHAR(lower + 'A' - 10);
  }
}

//Calls output_char in a loop.
void output_string(char *s) {
  int i = 0;
  while(s[i] != '\0') {
    OUTPUT_CHAR(s[i]);
    i++;
  }
}
