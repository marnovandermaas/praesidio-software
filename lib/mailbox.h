#ifndef PRAESIDIO_MAILBOX_HEADER
#define PRAESIDIO_MAILBOX_HEADER

#include "enclaveLibrary.h"

void sendMessage(struct Message_t *txMsg);
void receiveMessage(struct Message_t *rxMsg);

#endif
