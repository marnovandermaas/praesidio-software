#include "praesidioenclave.h"
#include "hello.h"

int main(char * output) {
  //This is the code that runs in the enclave world.
  volatile char *address;
  char read_buffer[INPUT_LEN+5];

  read_buffer[0] = 'H';
  read_buffer[1] = 'i';
  read_buffer[2] = ' ';
  read_buffer[3] = '\n';
  read_buffer[4] = '\0';

  if(give_read_permission(output, output, ENCLAVE_DEFAULT_ID)) {
    read_buffer[0] = 'N';
    read_buffer[1] = 'o';
  }
  output_string(read_buffer);

  address = get_receive_mailbox_base_address(ENCLAVE_DEFAULT_ID);

  for (int i = 0; i < NUMBER_OF_NAMES; i++) {
    output += send_enclave_message(output, read_buffer, OUTPUT_LEN);
    address += get_enclave_message(address, &read_buffer[3]);
    output_string(read_buffer);
    OUTPUT_CHAR('\n');
  }
}
