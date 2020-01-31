typedef unsigned long uint64_t; //TODO make sure it is 64-bit
#include <praesidioenclave.h>

int main() {
  char hello_string[64];
  hello_string[0] = 'H';
  hello_string[1] = 'i';
  hello_string[2] = '!';
  hello_string[3] = '\n';
  hello_string[4] = '\0';
  output_string(hello_string);
  // //This is the code that runs in the enclave world.
  // volatile char *address;
  // char *output;
  // asm volatile (
  //   "addi %0, a0, zero"
  //   : "=r"(output)
  //   :
  //   :
  // );
  // char read_buffer[INPUT_LEN+3];
  //
  // read_buffer[0] = 'H';
  // read_buffer[1] = 'i';
  // read_buffer[2] = ' ';
  //
  // give_read_permission(output, ENCLAVE_DEFAULT_ID);
  // address = get_receive_mailbox_base_address(ENCLAVE_DEFAULT_ID);
  //
  // for (int i = 0; i < NUMBER_OF_NAMES; i++) {
  //   address += get_enclave_message(address, &read_buffer[3]);
  //   output_string(read_buffer);
  //   output += send_enclave_message(output, read_buffer, OUTPUT_LEN);
  //   output_char('\n');
  // }
}
