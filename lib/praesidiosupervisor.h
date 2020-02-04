#ifndef PRAESIDIO_SUPERVISOR_HEADER
#define PRAESIDIO_SUPERVISOR_HEADER

#include "praesidio.h"

int give_read_permission(void* page_base, enclave_id_t receiver_id);

volatile void* get_receive_mailbox_base_address(enclave_id_t sender_id);

#endif //PRAESIDIO_SUPERVISOR_HEADER
