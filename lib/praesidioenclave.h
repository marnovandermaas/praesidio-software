#ifndef PRAESIDIO_ENCLAVE_HEADER
#define PRAESIDIO_ENCLAVE_HEADER

#include "praesidio.h"
#include "../riscv/encoding.h"

int give_read_permission(void* page_base, enclave_id_t receiver_id);

volatile void* get_receive_mailbox_base_address(enclave_id_t sender_id);

//Writes a character to display.
void output_char(char c);

//Writes hex of a byte to display.
void output_hexbyte(unsigned char c);

//Writes a whole string to display
void output_string(char *s);

#endif //PRAESIDIO_ENCLAVE_HEADER
