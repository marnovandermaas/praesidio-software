// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#ifndef PRAESIDIO_MAILBOX_HEADER
#define PRAESIDIO_MAILBOX_HEADER

#include "enclaveLibrary.h"

#define MAILBOX_VIRT_ADDRESS (ENCLAVE_VIRTUAL_ADDRESS_BASE)

void sendMessage(struct Message_t *txMsg);
void receiveMessage(struct Message_t *rxMsg);
void virtSendMessage(struct Message_t *txMsg);
void virtReceiveMessage(struct Message_t *rxMsg);

#endif
