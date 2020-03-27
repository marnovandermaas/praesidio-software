#include "praesidioenclave.h"
#include "communication.h"

//Sets read access to a page to an enclave
int give_read_permission(void *phys_page_base, void *virt_page_base, enclave_id_t receiver_id) {
  unsigned long page_number = ((unsigned long) phys_page_base - DRAM_BASE) >> PAGE_BIT_SHIFT;
  char *byte_base;
  if ((unsigned long) phys_page_base < DRAM_BASE) { //Check if pagebase is in DRAM
    return -1;
  }
  if ((((unsigned long) phys_page_base >> PAGE_BIT_SHIFT) << PAGE_BIT_SHIFT) != (unsigned long) phys_page_base) { //Check if lower bits are zero
    return -2;
  }
  byte_base = (char *) virt_page_base;
  byte_base[0] = BUSY_SIGNAL;
  SET_ARGUMENT_ENCLAVE_IDENTIFIER(receiver_id);
  asm volatile (
    "csrrw zero, 0x40A, %0"
    :
    : "r"(page_number)
    :
  );
  return 0;
}

//Gets base address of mailbox page from which you can receive messages from the enclave specified in sender_id.
volatile void* get_receive_mailbox_base_address(enclave_id_t sender_id) {
  volatile void *ret_val = 0;
  SET_ARGUMENT_ENCLAVE_IDENTIFIER(sender_id);
  do {
    asm volatile (
      "csrrs %0, 0x40B, zero"
      : "=r"(ret_val)
      :
      :
    );
  } while (ret_val == 0);
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
