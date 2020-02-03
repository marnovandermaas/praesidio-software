#ifndef PRAESIDIO_USER_HEADER
#define PRAESIDIO_USER_HEADER

#include <stdint.h>
#include "praesidio.h"

#define __NR_create_enclave       292
#define __NR_create_send_mailbox  293
#define __NR_get_receive_mailbox  294

void* NW_create_send_mailbox(enclave_id_t receiver_id);

volatile void* NW_get_receive_mailbox(enclave_id_t sender_id);

enclave_id_t start_enclave(void* enclave_memory);

#endif //PRAESIDIO_USER_HEADER
