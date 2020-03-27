#include "praesidiouser.h"
#include <sys/ioctl.h> //ioctl
#include <sys/mman.h> //mmap
#include <fcntl.h> //open
#include <unistd.h> //close
#include <errno.h> //errno
#include <string.h> //strerror
#include <stdio.h> //printf

char* NW_create_send_mailbox(int enclave_descriptor) {
  if (ioctl(enclave_descriptor, IOCTL_CREATE_SEND_MAILBOX) < 0) {
    printf("Error: failed to create send mailbox because %s.\n", strerror(errno));
    return NULL;
  }
  return (char *) mmap(NULL, 1 << PAGE_BIT_SHIFT, PROT_WRITE, MAP_SHARED, enclave_descriptor, 0);
}

char* NW_get_receive_mailbox(int enclave_descriptor) {
  if (ioctl(enclave_descriptor, IOCTL_GET_RECEIVE_MAILBOX) < 0) { //create send mailbox
    printf("Error: failed to get receive mailbox because %s.\n", strerror(errno));
    return NULL;
  }
  void *mapped_address = mmap(NULL, 1 << PAGE_BIT_SHIFT, PROT_WRITE, MAP_SHARED, enclave_descriptor, 0);
  if(mapped_address == MAP_FAILED) {
    return NULL;
  }
  return (char *) mapped_address;
}

int start_enclave(void *enclave_memory) {
  char device_name[128]; //TODO include 128 from driver header file.
  char device_path[128];
  int file_descriptor = open("/dev/praesidio", O_RDONLY);
  if(file_descriptor == -1) {
    printf("Failed to open main driver /dev/praesidiod because %s\n", strerror(errno));
    return errno;
  }
  ioctl(file_descriptor, 0, device_name);
  close(file_descriptor);
  sprintf(device_path, "/dev/%s", device_name);
  int enclave_descriptor = -1;
  printf("start_enclave: Opening enclave file %s", device_path);
  while(enclave_descriptor == -1) {
    enclave_descriptor = open(device_path, O_RDWR);
    printf(".");
  }
  printf("\n");
  if (ioctl(enclave_descriptor, IOCTL_CREATE_ENCLAVE, enclave_memory) < 0) { //create enclave: add enclave memory in here.
    printf("Error: failed create enclave ioctl.\n");
    return -1;
  }
  return enclave_descriptor;
}
