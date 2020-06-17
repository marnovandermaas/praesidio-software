#include "praesidioenclave.h"
#include "page.h"

int main(char * output) {
    //This is the code that runs in the enclave world.
    volatile char *address = 0;
    volatile long long *long_address = 0;
    unsigned int i, j;
    char start_buffer[ACK_LENGTH];
    start_buffer[0] = 'C';
    start_buffer[1] = '\0';

    if(give_read_permission(output, output, ENCLAVE_DEFAULT_ID)) { return -1; }

    address = get_receive_mailbox_base_address(ENCLAVE_DEFAULT_ID); //TODO move this to inside the loop once page donation is properly implemented.
    long_address = (volatile long long *) address;
    for(j = 0; j < PAGE_NUMBER_OF_REPEATS; j++) {
        while(long_address[0] != j); //This currently simulates the waiting for the normal world to donate a page to the enclave world.
        for(i = 1; i < (1 << PAGE_BIT_SHIFT) / sizeof(long long); i++) {
            if(long_address[i] != i) {
                start_buffer[0] = 'X';
                break;
            }
        }
        output += send_enclave_message(output, start_buffer, ACK_LENGTH);
        start_buffer[0]++;
    }
}
