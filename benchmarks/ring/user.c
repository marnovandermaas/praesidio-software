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
  char *rx_address = NULL;
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

  for (int packet_size = START_PACKET_SIZE; packet_size > 0; packet_size >>= 1) {
    printf("Packet size 0x%08x ", packet_size);
    for (int i = 0; i < packet_size-1; i++) {
      send_buffer[i] = fill_char;
    }
    send_buffer[packet_size-1] = '\0';
    fill_char ^= 1;

    //Ignoring first packet sent by them.
    tx_address += send_enclave_message(tx_address, send_buffer, packet_size);
    rx_address += get_enclave_message(rx_address, read_buffer);

    for (int i = 1; i < NUMBER_OF_REPS; i++) {
      OUTPUT_STATS(packet_size);
      tmp_length = send_enclave_message(tx_address, send_buffer, packet_size);
      OUTPUT_STATS(packet_size);
      tx_address += tmp_length;
      OUTPUT_STATS(packet_size);
      tmp_length = get_enclave_message(rx_address, read_buffer);
      OUTPUT_STATS(packet_size);
      rx_address += tmp_length;
      //printf("Got: %s\n", read_buffer);
      //printf("%d ", i);
    }
    printf("\n");
  }
  return 0;
}
