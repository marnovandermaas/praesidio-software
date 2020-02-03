#ifndef PRAESIDIO_INSTRUCTIONS_H
#define PRAESIDIO_INSTRUCTIONS_H

#include "enclaveLibrary.h"

void switchEnclaveID(enclave_id_t id);
//int* derivePhysicalCapability(struct PhysCap_t sourceCap);
CoreID_t getCoreID(void);
enclave_id_t getCurrentEnclaveID(void);
//This is a helper instruction, but will be removed in final design
void setArgumentEnclaveIdentifier(enclave_id_t id);
void sendMessage(struct Message_t *txMsg);
void receiveMessage(struct Message_t *rxMsg);

#endif //PRAESIDIO_INSTRUCTIONS_H
