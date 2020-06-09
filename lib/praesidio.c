#include "praesidio.h"
#include "communication.h"

//Returns the starting index for the next message.
int send_enclave_message(char *mailbox_address, char *message, int length) {
  int offset = LENGTH_SIZE + length;
  int i;

  if(length > ((2 << PAGE_BIT_SHIFT)-2*LENGTH_SIZE)) {
    return -1;
  }

  mailbox_address[0] = BUSY_BYTE;

  for(i = 0; i < length; i++) {
    mailbox_address[LENGTH_SIZE+i] = message[i];
  }

  mailbox_address[offset] = BUSY_BYTE;

  for(i = 0; i < LENGTH_SIZE; i++) {
    mailbox_address[LENGTH_SIZE-1-i] = ((length >> (8*i)) & 0xFF);
  }// mailbox_address[1] = length & 0xFF; mailbox_address[0] = 0;
  return offset;
}

//Returns the starting index for the next message
int get_enclave_message(volatile char *mailbox_address, char *read_buffer) {
  int i;
  int length = 0;

  while (mailbox_address[0] == BUSY_BYTE);

  for (i = 0; i < LENGTH_SIZE; i++) {
    length |= (((int) (mailbox_address[i] & 0xFF)) << (8*(LENGTH_SIZE-1-i)));
  }// length = mailbox_address[1] & 0xFF;

  for(i = 0; i < length; i++) {
    read_buffer[i] = mailbox_address[LENGTH_SIZE+i];
  }

  return LENGTH_SIZE + length;
}
