#include "praesidioenclave.h"
#include "ring.h"

int main(char * output) {
  //This is the code that runs in the enclave world.
  volatile char *address;
  int tmp_length = 0;
  char start_buffer[2];
  start_buffer[0] = 'S';
  start_buffer[1] = '\0';
  
  if(give_read_permission(output, output, ENCLAVE_DEFAULT_ID)) { }

  address = get_receive_mailbox_base_address(ENCLAVE_DEFAULT_ID);

  output +=send_enclave_message(output, start_buffer, 2);

  for (int packet_size = PACKET_START; packet_size <= PACKET_MAX; packet_size += PACKET_INCREMENT) {
    for (int i = 0; i < NUMBER_OF_REPS; i++) {
      tmp_length = get_enclave_message(address, output+LENGTH_SIZE);
      address += tmp_length;
      tmp_length = send_enclave_message(output, output+LENGTH_SIZE, packet_size);
      output += tmp_length;
    }
  }
}
