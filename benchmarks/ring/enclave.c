// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#include "praesidioenclave.h"
#include "ring.h"

int main(char * output) {
  //This is the code that runs in the enclave world.
  volatile char *address;
  int tmp_length = 0;
  char *buffer_address = output + (1 << PAGE_BIT_SHIFT);

  for(int i = 0; i < (1 << PAGE_BIT_SHIFT); i++) {
    buffer_address[i] = 'E';
  }

  setup_communication_pages(ENCLAVE_DEFAULT_ID, (void *) output, (volatile void **) &address);

  for (int packet_size = PACKET_START; packet_size <= PACKET_MAX; packet_size += PACKET_INCREMENT) {
    for (int i = 0; i < NUMBER_OF_REPS; i++) {
      tmp_length = send_enclave_message(output, buffer_address, packet_size);
      output += tmp_length;
      tmp_length = get_enclave_message(address, buffer_address);
      // OUTPUT_CHAR('E');
      // OUTPUT_CHAR(' ');
      // output_hexbyte((char) tmp_length);
      // OUTPUT_CHAR('\n');
      // for(int j = 0; j < packet_size; j++) {
      //     output_hexbyte(buffer_address[j]);
      //     OUTPUT_CHAR(' ');
      // }
      // OUTPUT_CHAR('\n');
      address += tmp_length;
    }
  }
}
