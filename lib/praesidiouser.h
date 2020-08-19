#ifndef PRAESIDIO_USER_HEADER
#define PRAESIDIO_USER_HEADER

#include <unistd.h>
#include <stdint.h>
#include "praesidiodriver.h"

char* get_send_page(int enclave_descriptor);

volatile char* get_read_only_page(int enclave_descriptor);

void setup_communication_pages(int enclave_descriptor, char **send_page, volatile char **receive_page);

int create_enclave(void *enclave_memory);

void delete_enclave(int enclave_descriptor);

//TODO return public key and signature
void attest_enclave(int enclave_descriptor, int nonce);

#endif //PRAESIDIO_USER_HEADER
