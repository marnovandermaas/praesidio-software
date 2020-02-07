#include <stdio.h>
#include "praesidiouser.h"

int main(void) {
  char *tx_address = NULL;
  printf("Starting driver test\n");
  tx_address = NW_create_send_mailbox(1);
  printf("Got address 0x%016lx\n", tx_address);
  // tx_address[0] = 1;
  // printf("Done.\n");
}
