#ifndef PRAESIDIO_SUPERVISOR_HEADER
#define PRAESIDIO_SUPERVISOR_HEADER

#include "praesidio.h"

int give_read_permission(void *phys_page_base, void *virt_page_base, enclave_id_t receiver_id);

volatile void* get_read_only_page(enclave_id_t sender_id);

int setup_communication_pages(enclave_id_t receiver_id, void *send_address, volatile void **receive_address);

#endif //PRAESIDIO_SUPERVISOR_HEADER
