// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#ifndef PRAESIDIO_ENCLAVE_HEADER
#define PRAESIDIO_ENCLAVE_HEADER

#include "unsignedinteger.h"
#include "instructions.h"
#include "praesidio.h"
#include "praesidiooutput.h"

void exit_enclave (void);

int give_read_permission(void *virt_page_base, enclave_id_t receiver_id);

volatile void* get_read_only_page(enclave_id_t sender_id);

int setup_communication_pages(enclave_id_t receiver_id, void *send_address, volatile void **receive_address);


#endif //PRAESIDIO_ENCLAVE_HEADER
