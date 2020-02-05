#include "praesidioenclave.h"
#include "hello.h"

int main(char * output) {
  //This is the code that runs in the enclave world.
  volatile char *address;
  char read_buffer[INPUT_LEN+3];

  read_buffer[0] = 'H';
  read_buffer[1] = 'i';
  read_buffer[2] = ' ';
  read_buffer[3] = '\0';
  output_string(read_buffer);

  give_read_permission(output, output, ENCLAVE_DEFAULT_ID);
  address = get_receive_mailbox_base_address(ENCLAVE_DEFAULT_ID);

  for (int i = 0; i < NUMBER_OF_NAMES; i++) {
    address += get_enclave_message(address, &read_buffer[3]);
    output_string(read_buffer);
    output += send_enclave_message(output, read_buffer, OUTPUT_LEN);
    output_char('\n');
  }
}
