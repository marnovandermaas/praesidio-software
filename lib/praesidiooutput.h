// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#ifndef PRAESIDIO_OUTPUT_HEADER
#define PRAESIDIO_OUTPUT_HEADER
//Writes a character to display.
//This instruction writes a value to a CSR register. This is a custom register and I have modified Spike to print whatever character is written to this CSR. This is my way of printing without requiring the support of a kernel or the RISC-V front end server.
#define OUTPUT_CHAR(c) ({asm volatile ("csrrw zero, 0x404, %0" : : "r"(c) : );})

//#define PRAESIDIO_DEBUG

//Writes hex of a byte to display.
void output_hexbyte(unsigned char c);

//Writes a whole string to display
void output_string(char *s);

#endif
