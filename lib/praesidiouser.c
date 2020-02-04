#include "praesidiouser.h"

void* NW_create_send_mailbox(enclave_id_t receiver_id) {
  return syscall(__NR_create_send_mailbox, receiver_id);
}

volatile void* NW_get_receive_mailbox(enclave_id_t sender_id) {
  return syscall(__NR_get_receive_mailbox, sender_id);
}

enclave_id_t start_enclave(void* enclave_memory) {
  //start_enclave_fast(NUMBER_OF_ENCLAVE_PAGES);
  return syscall(__NR_create_enclave, enclave_memory);
}
