// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

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

  enclave_descriptor = create_enclave((void*) enclave_memory_buffer);

  setup_communication_pages(enclave_descriptor, &tx_address, &rx_address);
  if(tx_address == NULL) {
    printf("Error setting up send mailbox.\n");
    return -1;
  }
  if(rx_address == NULL) {
    printf("Error getting receive mailbox.\n");
    return -1;
  }

  //volatile char read_aggregator;
  for (int packet_size = PACKET_START; packet_size <= PACKET_MAX; packet_size += PACKET_INCREMENT) {
    printf("Packet size 0x%08x ", packet_size);
    for (int i = 0; i < packet_size; i++) {
      send_buffer[i] = fill_char;
    }
    fill_char += 1;
    if(fill_char > 'Z') fill_char = 'A';

    for (int i = 0; i < NUMBER_OF_REPS; i++) {
      OUTPUT_STATS(packet_size);
      tmp_length = send_enclave_message(tx_address, send_buffer, packet_size);
      OUTPUT_STATS(packet_size);
      if(tmp_length == 0) {
        printf("Error send failed.\n");
        return -1;
      }
      tx_address += tmp_length;
      OUTPUT_STATS(packet_size);
      tmp_length = get_enclave_message(rx_address, read_buffer);
      OUTPUT_STATS(packet_size);
      if(tmp_length == 0) {
        printf("Error receive failed.\n");
        return -2;
      }
      rx_address += tmp_length;

      /*read_aggregator = 0x00;
      if(tmp_length < 0) { //In case the ring buffer runs past the page boundary
          tmp_length += (1 << PAGE_BIT_SHIFT);
      }
      for(int j = 0; j < tmp_length - LENGTH_SIZE; j++) {
        if(read_buffer[j] < 'A' || read_buffer[j] > 'Z') {
            printf("wrong char: 0x%x, %d, 0x%x\n", read_buffer[j], j, read_aggregator);
            return -3;
        }
        read_aggregator |= read_buffer[j];
      }*/
      //printf("(%d,%c) ", i, read_aggregator);
    }
    printf("\n");
  }
  return 0;
}
