// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "praesidiouser.h"

int main(void) {
  char device_name[128]; //TODO include 128 from driver header file.
  char device_path[128];
  printf("Starting driver test\n");
  int file_descriptor = open("/dev/praesidio", O_RDONLY);
  if(file_descriptor == -1) {
    printf("Failed to open main driver /dev/praesidiod because %s\n", strerror(errno));
    return errno;
  }
  ioctl(file_descriptor, 0, device_name);
  printf("Got name %s\n", device_name);
  close(file_descriptor);
  sprintf(device_path, "/dev/%s", device_name);
  int enclave_descriptor = -1;
  printf("Opening enclave file %s", device_path);
  while(enclave_descriptor == -1) {
    enclave_descriptor = open(device_path, O_RDWR);
    printf(".");
  }
  printf("\n");

  if (ioctl(enclave_descriptor, IOCTL_CREATE_ENCLAVE) < 0) { //create enclave: add enclave memory in here.
    printf("Error: failed create enclave ioctl.\n");
    return -1;
  }

  if (ioctl(enclave_descriptor, IOCTL_CREATE_SEND_MAILBOX) < 0) { //create send mailbox
    printf("Error: failed to create send mailbox because %s.\n", strerror(errno));
    return -1;
  }
  void *tx_ptr = mmap(NULL, 1 << PAGE_BIT_SHIFT, PROT_WRITE, MAP_SHARED, enclave_descriptor, 0);


  if (ioctl(enclave_descriptor, IOCTL_GET_RECEIVE_MAILBOX) < 0) { //create send mailbox
    printf("Error: failed to get receive mailbox because %s.\n", strerror(errno));
    return -1;
  }
  void *rx_ptr = mmap(NULL, 1 << PAGE_BIT_SHIFT, PROT_WRITE, MAP_SHARED, enclave_descriptor, 0);

  return 0;
}
