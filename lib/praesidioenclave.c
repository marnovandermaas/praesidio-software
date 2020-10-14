// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#include "praesidioenclave.h"
#include "communication.h"
#include "mailbox.h"

uint64_t __management_syscall(enum ManagementCall_t callType, void *argAddress, enclave_id_t argId) {
  asm volatile(
    "ecall"
    :
    :
    :
  );
}

//Sets read access to a page to an enclave
int give_read_permission(void *virt_page_base, enclave_id_t receiver_id) {
  unsigned long page_number;
  char *byte_base;
  struct Message_t message;

  if ((((unsigned long) virt_page_base >> PAGE_BIT_SHIFT) << PAGE_BIT_SHIFT) != (unsigned long) virt_page_base) { //Check if lower bits are zero
    return -2;
  }

  page_number = __management_syscall(MANAGE_SETREADER, virt_page_base, receiver_id);
  byte_base = (char *) virt_page_base;
  byte_base[0] = BUSY_BYTE;
  message.type = MSG_SHARE_PAGE;
  message.source = getCurrentEnclaveID();
  message.destination = receiver_id;
  message.arguments[0] = page_number;
  virtSendMessage(&message);
  return 0;
}

//Gets base address of mailbox page from which you can receive messages from the enclave specified in sender_id.
volatile void* get_read_only_page(enclave_id_t sender_id) {
  void *ret_val = 0;
  int i;
  enclave_id_t this_id = getCurrentEnclaveID();
  struct Message_t message;

  do {
    virtReceiveMessage(&message);
    if(message.type == MSG_SHARE_PAGE && message.source == sender_id && message.destination == this_id) {
        ret_val = (void *) (message.arguments[0] << PAGE_BIT_SHIFT) + DRAM_BASE;
    }
    for(i=0; i<100; i++);//delay a bit before asking again.
  } while (ret_val == 0);

  ret_val = (void *) __management_syscall(MANAGE_MAPMAIL, ret_val, ENCLAVE_INVALID_ID);

  return (volatile void *) ret_val;
}

int setup_communication_pages(enclave_id_t receiver_id, void *send_address, volatile void **receive_address) {
  int ret_val = 0;
  *receive_address = get_read_only_page(receiver_id);
  ret_val = give_read_permission(send_address, receiver_id);
  return ret_val;
}

void exit_enclave () {
  __management_syscall(MANAGE_EXIT, 0, ENCLAVE_INVALID_ID);
}
