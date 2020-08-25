#ifndef PRAESIDIO_INSTRUCTIONS_H
#define PRAESIDIO_INSTRUCTIONS_H

#include "enclaveLibrary.h"

#define SWITCH_ENCLAVE_ID(id) ({asm volatile ( "csrrw zero, 0x40E, %0" : : "r"(id) : );})

CoreID_t getCoreID(void);

enclave_id_t getCurrentEnclaveID(void);

//This is a helper instruction, but will be removed in final design
#define SET_ARGUMENT_ENCLAVE_IDENTIFIER(id) ({asm volatile ("csrrw zero, 0x40D, %0" : : "r"(id) : );})

uint64_t getCycleCount(void);

#endif //PRAESIDIO_INSTRUCTIONS_H
