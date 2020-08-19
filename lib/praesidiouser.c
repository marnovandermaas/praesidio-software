#include "praesidiouser.h"
#include <sys/ioctl.h> //ioctl
#include <sys/mman.h> //mmap
#include <fcntl.h> //open
#include <unistd.h> //close
#include <errno.h> //errno
#include <string.h> //strerror
#include <stdio.h> //printf

char* get_send_page(int enclave_descriptor) {
  if (ioctl(enclave_descriptor, IOCTL_CREATE_SEND_MAILBOX) < 0) {
    printf("Error: failed to create send mailbox because %s.\n", strerror(errno));
    return NULL;
  }
  return (char *) mmap(NULL, 1 << PAGE_BIT_SHIFT, PROT_WRITE, MAP_SHARED, enclave_descriptor, 0);
}

volatile char* get_read_only_page(int enclave_descriptor) {
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

void setup_communication_pages(int enclave_descriptor, char **send_page, volatile char **receive_page) {
  *send_page = get_send_page(enclave_descriptor);
  *receive_page = get_read_only_page(enclave_descriptor);
}

int create_enclave(void *enclave_memory) {
  char device_name[128]; //TODO include 128 from driver header file.
  char device_path[128];
  int label = 1000;
  int file_descriptor = -1;
  OUTPUT_STATS(label);
  file_descriptor = open("/dev/praesidio", O_RDONLY);
  if(file_descriptor == -1) {
    printf("Failed to open main driver /dev/praesidiod because %s\n", strerror(errno));
    return errno;
  }
  ioctl(file_descriptor, 0, device_name);
  close(file_descriptor);
  OUTPUT_STATS(label);
  sprintf(device_path, "/dev/%s", device_name);
  int enclave_descriptor = -1;
#ifdef PRAESIDIO_DEBUG
  printf("start_enclave: Opening enclave file %s", device_path);
#endif
  OUTPUT_STATS(label);
  while(enclave_descriptor == -1) {
    usleep(10);
    enclave_descriptor = open(device_path, O_RDWR);
#ifdef PRAESIDIO_DEBUG
    printf(".");
#endif
  }
  OUTPUT_STATS(label);
#ifdef PRAESIDIO_DEBUG
  printf("\n");
#endif
  if (ioctl(enclave_descriptor, IOCTL_CREATE_ENCLAVE, enclave_memory) < 0) { //create enclave: add enclave memory in here.
    printf("Error: failed create enclave ioctl.\n");
    return -1;
  }
  return enclave_descriptor;
}

void delete_enclave(int enclave_descriptor) {
  if (ioctl(enclave_descriptor, IOCTL_DELETE_ENCLAVE) < 0) {
    printf("Error: failed to delete enclave because %s.\n", strerror(errno));
  }
}

//TODO return public key and signature
void attest_enclave(int enclave_descriptor, int nonce) {
  if (ioctl(enclave_descriptor, IOCTL_ATTEST_ENCLAVE) < 0) {
    printf("Error: failed to attest enclave because %s.\n", strerror(errno));
  }
}
