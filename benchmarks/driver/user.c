#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
//#include "praesidiouser.h"

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
    enclave_descriptor = open(name2, O_RDWR);
    printf(".");
  }
  printf("\n");
  ioctl(enclave_descriptor, 11, 13);
  return 0;
}
