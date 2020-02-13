#ifndef PRAESIDIO_DRIVER_HEADER
#define PRAESIDIO_DRIVER_HEADER

#include <linux/types.h>

#include "praesidio.h"
#include "instructions.h"

#define IOCTL_CREATE_ENCLAVE _IO('a', 1)
#define IOCTL_CREATE_SEND_MAILBOX _IO('a', 2)
#define IOCTL_GET_RECEIVE_MAILBOX _IO('a', 3)

long NW_create_send_mailbox(void *tx_address, enclave_id_t receiver_id);

long NW_get_receive_mailbox(void *rx_address, enclave_id_t sender_id);

enclave_id_t start_enclave(void* enclave_memory);

#endif //PRAESIDIO_DRIVER_HEADER
