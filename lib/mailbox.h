// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#ifndef PRAESIDIO_MAILBOX_HEADER
#define PRAESIDIO_MAILBOX_HEADER

#include "enclaveLibrary.h"

void sendMessage(struct Message_t *txMsg);
void receiveMessage(struct Message_t *rxMsg);

#endif
