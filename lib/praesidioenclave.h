#ifndef PRAESIDIO_ENCLAVE_HEADER
#define PRAESIDIO_ENCLAVE_HEADER

#include "unsignedinteger.h"
#include "instructions.h"
#include "praesidiosupervisor.h"

//Writes a character to display.
void output_char(char c);

//Writes hex of a byte to display.
void output_hexbyte(unsigned char c);

//Writes a whole string to display
void output_string(char *s);

#endif //PRAESIDIO_ENCLAVE_HEADER
