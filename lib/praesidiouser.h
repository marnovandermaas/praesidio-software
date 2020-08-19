#ifndef PRAESIDIO_USER_HEADER
#define PRAESIDIO_USER_HEADER

#include <unistd.h>
#include <stdint.h>
#include "praesidiodriver.h"

char* NW_create_send_mailbox(int enclave_descriptor);

char* NW_get_receive_mailbox(int enclave_descriptor);

int start_enclave(void *enclave_memory);

#endif //PRAESIDIO_USER_HEADER
