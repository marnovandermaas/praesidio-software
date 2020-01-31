#ifndef PRAESIDIO_HEADER
#define PRAESIDIO_HEADER

#include "management.h"
#include "instructions.h"

#define NUMBER_OF_ENCLAVE_PAGES       (2)
#define NUMBER_OF_STACK_PAGES         (4)
#define NUMBER_OF_COMMUNICATION_PAGES (2)

#define PAGE_TO_POINTER(NUM)          ((NUM << PAGE_BIT_SHIFT) | DRAM_BASE)

//Sends message of length and encodes it at mailbox address. It returns the amount the mailbox_address should be increased by for the next message.
int send_enclave_message(char *mailbox_address, char *message, int length);

//Gets message from mailbox_address and puts it into the read buffer. It returns the amount the mailbox_address should be increased by for the next message.
int get_enclave_message(volatile char *mailbox_address, char *read_buffer);

#endif //PRAESIDIO_HEADER
