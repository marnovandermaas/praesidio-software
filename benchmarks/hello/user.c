// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#include <stdio.h>
#include <stdlib.h>
#include "praesidiouser.h"
#include "hello.h"

int main(void)
{
  FILE *fp = NULL;
  int c;
  size_t i = 0;
  int enclave_descriptor;
  char name[INPUT_LEN] = "Marno";
  volatile char *rx_address = NULL;
  char *tx_address = NULL;
  char read_buffer[OUTPUT_LEN];
  char *enclave_memory_buffer = NULL;
  int label = NUMBER_OF_NAMES;
  int label2 = label + 1;
  int tmp_length = 0;

  OUTPUT_STATS(label2);
  OUTPUT_STATS(label);
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

  //printf("Copied %lu bytes to enclave memory buffer.\n", i);
  //printf("location of fp 0x%016lx, rx_address 0x%016lx, enclave_memory_buffer 0x%016lx\n", &fp, &rx_address, &enclave_memory_buffer);
  OUTPUT_STATS(label);

  OUTPUT_STATS(label);
  enclave_descriptor = create_enclave((void*) enclave_memory_buffer);
  OUTPUT_STATS(label);

  OUTPUT_STATS(label);
  setup_communication_pages(enclave_descriptor, &tx_address, &rx_address);
  if(tx_address == NULL) {
    printf("Error setting up send mailbox.\n");
    return -1;
  }
  if(rx_address == NULL) {
    printf("Error getting receive mailbox.\n");
    return -1;
  }
  OUTPUT_STATS(label);
  OUTPUT_STATS(label2);

  //printf("Received tx address: 0x%016lx\n", tx_address);
  //printf("Received rx address: 0x%016lx\n", rx_address);

  for (int i = 0; i < NUMBER_OF_NAMES; i++) {
    OUTPUT_STATS(i);
    tmp_length = send_enclave_message(tx_address, name, INPUT_LEN);
    OUTPUT_STATS(i);
    /*for(int j = 0; j < 10; j++) {
      printf("0x%x ", tx_address[j]);
    }
    printf("send length: %d %s\n", tmp_length, name);*/
    tx_address += tmp_length;
    OUTPUT_STATS(i);
    tmp_length = get_enclave_message(rx_address, read_buffer);
    OUTPUT_STATS(i);

    /*for(int j = 0; j < 10; j++) {
      printf("0x%x ", rx_address[j]);
    }
    printf("get length: %d\n", tmp_length);*/
    rx_address += tmp_length;
    printf("Got: %s\n", read_buffer);
    name[0]+=1;
  }
  return 0;
}
