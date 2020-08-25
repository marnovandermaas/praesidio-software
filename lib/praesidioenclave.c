#include "praesidioenclave.h"
#include "communication.h"
#include "mailbox.h"

//Sets read access to a page to an enclave
int give_read_permission(void *phys_page_base, void *virt_page_base, enclave_id_t receiver_id) {
  unsigned long page_number = ((unsigned long) phys_page_base - DRAM_BASE) >> PAGE_BIT_SHIFT;
  char *byte_base;
  struct Message_t message;
  if ((unsigned long) phys_page_base < DRAM_BASE) { //Check if pagebase is in DRAM
    return -1;
  }
  if ((((unsigned long) phys_page_base >> PAGE_BIT_SHIFT) << PAGE_BIT_SHIFT) != (unsigned long) phys_page_base) { //Check if lower bits are zero
    return -2;
  }
  byte_base = (char *) virt_page_base;
  byte_base[0] = BUSY_BYTE;
  SET_ARGUMENT_ENCLAVE_IDENTIFIER(receiver_id);
  asm volatile (
    "csrrw zero, 0x40A, %0"
    :
    : "r"(page_number)
    :
  );
  message.type = MSG_SHARE_PAGE;
  message.source = getCurrentEnclaveID();
  message.destination = receiver_id;
  message.arguments[0] = page_number;
  sendMessage(&message);
  return 0;
}

//Gets base address of mailbox page from which you can receive messages from the enclave specified in sender_id.
volatile void* get_read_only_page(enclave_id_t sender_id) {
  volatile void *ret_val = 0;
  int i;
  enclave_id_t this_id = getCurrentEnclaveID();
  struct Message_t message;
  do {
    receiveMessage(&message);
    if(message.type == MSG_SHARE_PAGE && message.source == sender_id && message.destination == this_id) {
        ret_val = (volatile void*) (message.arguments[0] << PAGE_BIT_SHIFT) + DRAM_BASE;
    }
    for(i=0; i<100; i++);//delay a bit before asking again.
  } while (ret_val == 0);
  return ret_val;
}

int setup_communication_pages(enclave_id_t receiver_id, void *send_address, volatile void **receive_address) {
  int ret_val = 0;
  *receive_address = get_read_only_page(receiver_id);
  ret_val = give_read_permission(send_address, send_address, receiver_id);
  return ret_val;
}

void output_hexbyte(unsigned char c) {
  unsigned char upper = (c >> 4);
  unsigned char lower = (c & 0xF);
  if(upper < 10) {
    OUTPUT_CHAR(upper + '0');
  } else {
    OUTPUT_CHAR(upper + 'A' - 10);
  }
  if(lower < 10) {
    OUTPUT_CHAR(lower + '0');
  } else {
    OUTPUT_CHAR(lower + 'A' - 10);
  }
}

//Calls output_char in a loop.
void output_string(char *s) {
  int i = 0;
  while(s[i] != '\0') {
    OUTPUT_CHAR(s[i]);
    i++;
  }
}
