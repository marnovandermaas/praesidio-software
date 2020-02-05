#include "praesidiouser.h"

long NW_create_send_mailbox(void *tx_address, enclave_id_t receiver_id) {
  return syscall(__NR_create_send_mailbox, tx_address, receiver_id);
}

long NW_get_receive_mailbox(void *rx_address, enclave_id_t sender_id) {
  return syscall(__NR_get_receive_mailbox, rx_address, sender_id);
}

enclave_id_t start_enclave(void* enclave_memory) {
  //start_enclave_fast(NUMBER_OF_ENCLAVE_PAGES);
  return syscall(__NR_create_enclave, enclave_memory);
}
