#include "unsignedinteger.h"
#include "instructions.h"

CoreID_t getCoreID() {
  CoreID_t id;
  asm volatile( //This function reads out a system register and put the ID of the core into the id variable
    "csrrs %0, 0xF14, zero" //CSSRS rd, mhartid, 0
    : "=r"(id) //output
    : //input
    : //clobbered
  );
  return id;
}

enclave_id_t getCurrentEnclaveID() {
  enclave_id_t retVal;
  asm volatile (
    "csrrs %0, 0x40E, zero"
    : "=r"(retVal) //output
    : //input
    : //clobbered
  );
  return retVal;
}

uint64_t getCycleCount() {
    uint64_t retVal;
    asm volatile (
      "rdcycle %0"
      : "=r"(retVal) //output
      : //input
      : //clobbered
    );
    return retVal;
}
