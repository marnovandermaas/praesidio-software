#ifndef PRAESIDIO_DRIVER_HEADER
#define PRAESIDIO_DRIVER_HEADER

#include <linux/types.h>

#include "praesidio.h"
#include "instructions.h"

#define IOCTL_CREATE_ENCLAVE _IOW('a', 1, void *)
#define IOCTL_CREATE_SEND_MAILBOX _IO('a', 2)
#define IOCTL_GET_RECEIVE_MAILBOX _IO('a', 3)

char* NW_create_send_mailbox(int enclave_descriptor);

char* NW_get_receive_mailbox(int enclave_descriptor);

int start_enclave(void *enclave_memory);

#endif //PRAESIDIO_DRIVER_HEADER
