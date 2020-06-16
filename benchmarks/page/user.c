#include <stdio.h>
#include <stdlib.h>
#include "praesidiouser.h"
#include "page.h"

int main(void)
{
    FILE *fp = NULL;
    int c;
    size_t i = 0;
    int enclave_descriptor;
    char *tx_address = NULL;
    volatile char *rx_address = NULL;
    long long *long_address = NULL;
    char *enclave_memory_buffer = NULL;
    int label = 0xBABE;

    char read_buffer[ACK_LENGTH];
    read_buffer[0] = '\0';

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

    OUTPUT_STATS(label);
    tx_address = NW_create_send_mailbox(enclave_descriptor);
    if(tx_address == NULL) {
        printf("Error setting up send mailbox.\n");
        return -1;
    }
    long_address = (long long *) tx_address;
    for(i = 0; i < (1 << PAGE_BIT_SHIFT) / sizeof(long long); i++) {
        long_address[i] = i;
    }
    
    rx_address = NW_get_receive_mailbox(enclave_descriptor);
    if(rx_address == NULL) {
        printf("Error getting receive mailbox.\n");
        return -1;
    }

    get_enclave_message(rx_address, read_buffer);
    OUTPUT_STATS(label);

    printf("%s\n", read_buffer);

    return 0;
}
