#include "praesidio.h"
#include "communication.h"

//Returns the starting index for the next message.
int send_enclave_message(char *mailbox_address, char *message, int length) {
  int current_offset = ((long long) mailbox_address) & ((1 << PAGE_BIT_SHIFT) - 1);
  int offset = LENGTH_SIZE + length;
  int i, index;

  if(length > ((1 << PAGE_BIT_SHIFT) - 2*LENGTH_SIZE)) {
    return 0;
  }

  mailbox_address[0] = BUSY_BYTE;

  for(i = 0; i < length; i++) {
    index = LENGTH_SIZE + i;
    if((current_offset + index) >= (1 << PAGE_BIT_SHIFT)) {
      index -= (1 << PAGE_BIT_SHIFT);
    }
    mailbox_address[index] = message[i];
  }

  if((current_offset + offset) >= (1 << PAGE_BIT_SHIFT)) {
    offset -= (1 << PAGE_BIT_SHIFT);
  }

  mailbox_address[offset] = BUSY_BYTE;

  for(i = 0; i < LENGTH_SIZE; i++) {
    index = LENGTH_SIZE-1-i;
    if((current_offset + index) >= (1 << PAGE_BIT_SHIFT)) {
      index -= (1 << PAGE_BIT_SHIFT);
    }
    mailbox_address[index] = ((length >> (8*i)) & 0xFF);
  }
  return offset;
}

//Returns the starting index for the next message
int get_enclave_message(volatile char *mailbox_address, char *read_buffer) {//TODO return the length of the received message as well
  int current_offset = ((long long) mailbox_address) & ((1 << PAGE_BIT_SHIFT) - 1);
  int i, index;
  int length = 0;
  int ret_val = 0;

  while (mailbox_address[0] == BUSY_BYTE);

  for (i = 0; i < LENGTH_SIZE; i++) {
    index = i;
    if((current_offset + index) >= (1 << PAGE_BIT_SHIFT)) {
      index -= (1 << PAGE_BIT_SHIFT);
    }
    length |= (((int) (mailbox_address[index] & 0xFF)) << (8*(LENGTH_SIZE-1-i)));
  }

  if (length > ((1 << PAGE_BIT_SHIFT) - 2*LENGTH_SIZE)) {
    return 0;
  }

  for(i = 0; i < length; i++) {
    index = LENGTH_SIZE + i;
    if((current_offset + index) >= (1 << PAGE_BIT_SHIFT)) {
      index -= (1 << PAGE_BIT_SHIFT);
    }
    read_buffer[i] = mailbox_address[index];
  }

  ret_val = LENGTH_SIZE + length;
  if((current_offset + ret_val) >= (1 << PAGE_BIT_SHIFT)) {
    ret_val -= (1 << PAGE_BIT_SHIFT);
  }
  return ret_val;
}
