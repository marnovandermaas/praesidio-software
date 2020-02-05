#include <stdio.h>
#include <stdlib.h>
#include "praesidiouser.h"
#include "hello.h"

int main(void)
{
  FILE *fp = fopen("enclave.bin", "r");
  int c;
  size_t i = 0;
  enclave_id_t myenclave;
  char name[INPUT_LEN] = "Marno";
  char *rx_address = NULL;
  char *tx_address = malloc(1 << PAGE_BIT_SHIFT);
  char read_buffer[OUTPUT_LEN];
  char *enclave_memory_buffer = malloc(NUMBER_OF_ENCLAVE_PAGES << PAGE_BIT_SHIFT);

  for(i = 0; i < (NUMBER_OF_ENCLAVE_PAGES << PAGE_BIT_SHIFT); i++) {
    c = fgetc(fp);
    if( feof(fp) ) {
       break ;
    }
    enclave_memory_buffer[i] = (char) c;
  }
  printf("Copied %lu bytes to enclave memory buffer.\n", i);
  //Fill rest of memory with zeroes.
  for(; i < (NUMBER_OF_ENCLAVE_PAGES << PAGE_BIT_SHIFT); i++) {
    enclave_memory_buffer[i] = 0;
  }

  myenclave = start_enclave((void*) enclave_memory_buffer);
  printf("Got Enclave ID: %d\n", myenclave);
  fclose(fp);

  if(NW_create_send_mailbox(tx_address, myenclave)) {
    printf("Error setting up send mailbox.\n");
  }

  if(NW_get_receive_mailbox(rx_address, myenclave)) {
    printf("Error getting receive mailbox.\n");
  }

  for (int i = 0; i < NUMBER_OF_NAMES; i++) {
    tx_address += send_enclave_message(tx_address, name, INPUT_LEN);
    rx_address += get_enclave_message(rx_address, read_buffer);
    printf("Got: %s\n", read_buffer);
    name[0]+=1;
  }
}
