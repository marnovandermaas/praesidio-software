#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//#include "praesidiouser.h"

int main(void) {
  char name[128]; //TODO include 128 from driver header file.
  printf("Starting driver test\n");
  int file_descriptor = open("/dev/praesidio", O_RDWR);
  if(file_descriptor == -1) {
    printf("Failed to open main driver %s\n", name);
    return -1;
  }
  ioctl(file_descriptor, 0, name);
  printf("Got name %s\n", name);
}
