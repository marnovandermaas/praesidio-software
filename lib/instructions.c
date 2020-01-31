typedef unsigned long uint64_t; //TODO make sure it is 64-bit

#include "instructions.h"

void switchEnclaveID(enclave_id_t id) {
  asm volatile (
    "csrrw zero, 0x40E, %0"
    : //output
    : "r"(id) //input
    : //clobbered
  );
}

// int* derivePhysicalCapability(struct PhysCap_t sourceCap) {
//   //TODO add inline instruction code here
// }
//
// void startNormalWorld() {
//   //TODO add inline instruction to signal the normal world to start here
// }

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

// struct PhysCap_t getRootCapability() {
//   //TODO add inline instruction here
// }
//
// void setManagementInterruptTimer(int milis) {
//   //TODO add inline instruction to set the interupt time to milis amount of miliseconds.
// }

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

void setArgumentEnclaveIdentifier(enclave_id_t id) {
  //Set the enclave ID argument for the next instruction.
  asm volatile (
    "csrrw zero, 0x40D, %0"
    :
    : "r"(id)
    :
  );
}

void sendMessage(struct Message_t *txMsg) {
  //TODO
  unsigned long contentAndType = 0;
  setArgumentEnclaveIdentifier(txMsg->destination);
  contentAndType += (txMsg->content << SIZE_OF_MESSAGE_TYPE);
  contentAndType += (txMsg->type & MSG_MASK); //The and is unecessary assuming everything goes well.
  asm volatile (
    "csrrw zero, 0x410, %0"
    : //output
    : "r" (contentAndType) //input
    : //clobbered
  );
}

void receiveMessage(struct Message_t *rxMsg) {
  enclave_id_t source = ENCLAVE_INVALID_ID;
  unsigned long contentAndType;
  asm volatile (
    "csrrs %0, 0x411, zero"
    : "=r"(contentAndType) //output
    : //input
    : //clobbered
  );
  //Get the source enclave ID argument for the message.
  asm volatile (
    "csrrs %0, 0x40D, zero"
    : "=r"(source) //output
    : //input
    :
  );
  rxMsg->type = contentAndType & MSG_MASK;
  rxMsg->source = source;
  rxMsg->destination = getCurrentEnclaveID();
  rxMsg->content = ((unsigned long)(contentAndType - rxMsg->type)) >> SIZE_OF_MESSAGE_TYPE; //Assuming unsigned shift.
}
