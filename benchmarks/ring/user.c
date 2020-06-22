#include <stdio.h>
#include <stdlib.h>
#include "praesidiouser.h"
#include "ring.h"

int main(void)
{
  FILE *fp = NULL;
  int c;
  size_t i = 0;
  int enclave_descriptor;
  volatile char *rx_address = NULL;
  char *tx_address = NULL;
  char *enclave_memory_buffer = NULL;
  char *read_buffer = NULL;
  char *send_buffer = NULL;
  int tmp_length = 0;
  char fill_char = 'A';

  read_buffer = malloc(1 << PAGE_BIT_SHIFT);
  send_buffer = malloc(1 << PAGE_BIT_SHIFT);

  fp = fopen("enclave.bin", "r");
  enclave_memory_buffer = malloc(NUMBER_OF_ENCLAVE_PAGES << PAGE_BIT_SHIFT);
  for(i = 0; i < (NUMBER_OF_ENCLAVE_PAGES << PAGE_BIT_SHIFT); i++) {
    c = fgetc(fp);
    if( feof(fp) ) {
       break ;
    }
    enclave_memory_buffer[i] = (char) c;
  }
  fclose(fp);
  //Fill rest of memory with zeroes.
  for(; i < (NUMBER_OF_ENCLAVE_PAGES << PAGE_BIT_SHIFT); i++) {
    enclave_memory_buffer[i] = 0;
  }

  enclave_descriptor = start_enclave((void*) enclave_memory_buffer);

  tx_address = NW_create_send_mailbox(enclave_descriptor);
  if(tx_address == NULL) {
    printf("Error setting up send mailbox.\n");
    return -1;
  }

  rx_address = NW_get_receive_mailbox(enclave_descriptor);
  if(rx_address == NULL) {
    printf("Error getting receive mailbox.\n");
    return -1;
  }

  volatile char read_aggregator;
  for (int packet_size = PACKET_START; packet_size <= PACKET_MAX; packet_size += PACKET_INCREMENT) {
    printf("Packet size 0x%08x ", packet_size);
    for (int i = 0; i < packet_size-1; i++) {
      send_buffer[i] = fill_char;
    }
    fill_char += 1;
    if(fill_char == 'Z') fill_char = 'A';

    for (int i = 0; i < NUMBER_OF_REPS; i++) {
      OUTPUT_STATS(packet_size);
      tmp_length = send_enclave_message(tx_address, send_buffer, packet_size);
      OUTPUT_STATS(packet_size);
      tx_address += tmp_length;
      OUTPUT_STATS(packet_size);
      tmp_length = get_enclave_message(rx_address, read_buffer);
      OUTPUT_STATS(packet_size);
      rx_address += tmp_length;

      read_aggregator = 0x00;
      for(int j = 0; j < tmp_length - LENGTH_SIZE; j++) {
        //printf("%c", read_buffer[j]);
        read_aggregator |= read_buffer[j];
      }
      printf("(%d,%c) ", i, read_aggregator);
      //printf("\n");
    }
    printf("\n");
  }
  return 0;
}
