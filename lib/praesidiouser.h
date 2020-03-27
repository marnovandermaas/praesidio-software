#ifndef PRAESIDIO_USER_HEADER
#define PRAESIDIO_USER_HEADER

#include <unistd.h>
#include <stdint.h>
#include "praesidiodriver.h"

#define OUTPUT_STATS(input) ({asm volatile ("csrrw zero, 0x406, %0" : : "r"(input) : );})

#endif //PRAESIDIO_USER_HEADER
