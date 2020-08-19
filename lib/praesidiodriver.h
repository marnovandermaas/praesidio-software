#ifndef PRAESIDIO_DRIVER_HEADER
#define PRAESIDIO_DRIVER_HEADER

#include <linux/types.h>

#include "praesidio.h"
#include "instructions.h"

#define IOCTL_CREATE_ENCLAVE _IOW('a', 1, void *)
#define IOCTL_CREATE_SEND_MAILBOX _IO('a', 2)
#define IOCTL_GET_RECEIVE_MAILBOX _IO('a', 3)
#define IOCTL_ATTEST_ENCLAVE _IOW('a', 4, unsigned long)
#define IOCTL_DELETE_ENCLAVE _IO('a', 5)

#define OUTPUT_STATS(input) ({asm volatile ("csrrw zero, 0x406, %0" : : "r"(input) : );})

#endif //PRAESIDIO_DRIVER_HEADER
