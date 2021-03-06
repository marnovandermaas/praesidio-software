// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#include "praesidioenclave.h"
#include "hello.h"

int main(char * output) {
  //This is the code that runs in the enclave world.
  volatile char *address;
  char read_buffer[INPUT_LEN+5];
  int tmp_length = 0;

  read_buffer[0] = 'H';
  read_buffer[1] = 'i';
  read_buffer[2] = ' ';
  read_buffer[3] = '\n';
  read_buffer[4] = '\0';

  if(setup_communication_pages(ENCLAVE_DEFAULT_ID, (void *) output, (volatile void **) &address)) {
    read_buffer[0] = 'N';
    read_buffer[1] = 'o';
  }
  output_string(read_buffer);

  for (int i = 0; i < NUMBER_OF_NAMES; i++) {
    tmp_length = send_enclave_message(output, read_buffer, OUTPUT_LEN);
    /*for (int j = 0; j < 10; j++) {
      output_hexbyte(output[j]);
    }
    OUTPUT_CHAR('\n');*/
    output += tmp_length;
    tmp_length = get_enclave_message(address, &read_buffer[3]);
    /*for (int j = 0; j < 10; j++) {
      output_hexbyte(address[j]);
    }*/
    address += tmp_length;
    output_string(read_buffer);
    OUTPUT_CHAR('\n');
  }
}
