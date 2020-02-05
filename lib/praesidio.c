#include "praesidio.h"
#include "communication.h"

//Returns the starting index for the next message.
int send_enclave_message(char *mailbox_address, char *message, int length) {
  int offset = 0;
  int i;

  mailbox_address[offset] = BUSY_SIGNAL;
  offset += 1;

  for(i = 0; i < sizeof(length); i++) {
    mailbox_address[offset+i] = ((length >> (8*(sizeof(length)-1-i))) & 0xFF);
  }
  offset += sizeof(length);

  for(i = 0; i < length; i++) {
    mailbox_address[offset+i] = message[i];
  }
  offset += length;

  mailbox_address[offset] = BUSY_SIGNAL;
  mailbox_address[0] = READY_SIGNAL;
  return offset;
}

//Returns the starting index for the next message
int get_enclave_message(volatile char *mailbox_address, char *read_buffer) {
  int offset = 0;
  int i;
  int length = 0;

  while (mailbox_address[offset] != READY_SIGNAL);
  offset += 1;

  for (i = 0; i < sizeof(length); i++) {
    length |= (((int) mailbox_address[offset+i]) << (8*(sizeof(length)-1-i)));
  }
  offset += sizeof(length);

  for(i = 0; i < length; i++) {
    read_buffer[i] = mailbox_address[offset+i];
  }
  offset += length;

  return offset;
}
