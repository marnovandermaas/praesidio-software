#include <stdio.h>
#include <stdlib.h>
#include "praesidiouser.h"
#include "hello.h"

int main(void)
{
  FILE *fp = fopen("enclave.bin", "r");
  int c;
  size_t i = 0;
  int enclave_descriptor;
  char name[INPUT_LEN] = "Marno";
  char *rx_address = NULL;
  char *tx_address = NULL;
  char read_buffer[OUTPUT_LEN];
  char *enclave_memory_buffer = malloc(NUMBER_OF_ENCLAVE_PAGES << PAGE_BIT_SHIFT);

  bare_metal_stats();
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

  printf("location of fp 0x%016lx, rx_address 0x%016lx, enclave_memory_buffer 0x%016lx\n", &fp, &rx_address, &enclave_memory_buffer);

  enclave_descriptor = start_enclave((void*) enclave_memory_buffer);
  fclose(fp);

  tx_address = NW_create_send_mailbox(enclave_descriptor);
  if(tx_address == NULL) {
    printf("Error setting up send mailbox.\n");
    return -1;
  }
  printf("Received tx address: 0x%016lx\n", tx_address);

  rx_address = NW_get_receive_mailbox(enclave_descriptor);
  if(rx_address == NULL) {
    printf("Error getting receive mailbox.\n");
    return -1;
  }
  printf("Received rx address: 0x%016lx\n", rx_address);

  for (int i = 0; i < NUMBER_OF_NAMES; i++) {
    tx_address += send_enclave_message(tx_address, name, INPUT_LEN);
    rx_address += get_enclave_message(rx_address, read_buffer);
    printf("Got: %s\n", read_buffer);
    name[0]+=1;
  }
  return 0;
}
