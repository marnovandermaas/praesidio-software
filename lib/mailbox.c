#include "unsignedinteger.h"
#include "instructions.h"
//#include "praesidiooutput.h"
#include "mailbox.h"

void sendMessage(struct Message_t *txMsg) {
  volatile struct Message_t *myMailbox = (struct Message_t *) MAILBOX_BASE; //Processor will automatically write to the correct mailbox.
  myMailbox->type = MSG_INVALID; //Mark this mailbox as invalid while writing.
  //TODO insert fence here
  myMailbox->source = getCurrentEnclaveID();
  myMailbox->destination = txMsg->destination;
  myMailbox->arguments[0] = txMsg->arguments[0];
  myMailbox->arguments[1] = txMsg->arguments[1];
  //TODO insert fence here
  myMailbox->type = txMsg->type;
}

void receiveMessage(struct Message_t *rxMsg) {
  volatile struct Message_t *currentMessage;
  enclave_id_t enclave_id = getCurrentEnclaveID();
  unsigned int number_of_mailboxes = MAILBOX_SIZE / sizeof(struct Message_t);
  unsigned int i = 0;
  enum MessageType_t myType;
  enclave_id_t myDestination;
  for(i = 0; i < number_of_mailboxes; i++) {
    currentMessage = (volatile struct Message_t *) (MAILBOX_BASE + (i*sizeof(struct Message_t)));
    myType = currentMessage->type;
    myDestination = currentMessage->destination;
    if(myType < MSG_INVALID && myDestination == enclave_id) {
// #ifdef PRAESIDIO_DEBUG
//       output_string("mailbox.c: Got message in box 0x");
//       output_hexbyte(i);
//       output_string(" type 0x");
//       output_hexbyte((char) myType);
//       OUTPUT_CHAR('\n');
// #endif
      rxMsg->type = myType;
      rxMsg->source = currentMessage->source;
      rxMsg->destination = myDestination;
      rxMsg->arguments[0] = currentMessage->arguments[0];
      rxMsg->arguments[1] = currentMessage->arguments[1];
      return;
    }
  }
  rxMsg->type = MSG_INVALID;
  rxMsg->source = ENCLAVE_INVALID_ID;
  rxMsg->destination = ENCLAVE_INVALID_ID;
}
