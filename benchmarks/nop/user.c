// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#include <stdio.h>
#include <stdlib.h>
#include "praesidiouser.h"

int main(void)
{
  FILE *fp = NULL;
  char *enclave_memory_buffer = NULL;
  size_t i;
  int c;
  int enclave_descriptor;

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

  enclave_descriptor = create_enclave((void*) enclave_memory_buffer);
}
