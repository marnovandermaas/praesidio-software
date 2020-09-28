// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#ifndef PRAESIDIO_INSTRUCTIONS_H
#define PRAESIDIO_INSTRUCTIONS_H

#include "enclaveLibrary.h"

#define SWITCH_ENCLAVE_ID(id) ({asm volatile ( "csrrw zero, 0x40E, %0" : : "r"(id) : );})

CoreID_t getCoreID(void);

enclave_id_t getCurrentEnclaveID(void);

uint64_t getCycleCount(void);

#endif //PRAESIDIO_INSTRUCTIONS_H
