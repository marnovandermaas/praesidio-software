// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#include <sys/wait.h> /* wait */
#include <stdio.h>
#include <stdlib.h>   /* exit functions */
#include <unistd.h>   /* read, write, pipe, _exit */
#include <string.h>
#include "praesidiopage.h"
#include "praesidiouser.h"

#define ReadEnd  (0)
#define WriteEnd (1)
#define BytesToSend (1 << PAGE_BIT_SHIFT)
#define AckLength (3)
#define NumberReps (32)

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1);    /** failure **/
}

int main() {
  int bigPipe[2]; /* two file descriptors */
  int ackPipe[2];
  char* buf = NULL; /* pointer to bytes to write */
  char ack[AckLength] = "\0";
  int label = 0xBABE;

  if (pipe(bigPipe) < 0 || pipe(ackPipe)) report_and_exit("pipe");
  pid_t cpid = fork();                                /* fork a child process */
  if (cpid < 0) report_and_exit("fork");              /* check for failure */

  if (0 == cpid) {    /*** child ***/                 /* child process */
    close(bigPipe[WriteEnd]);                         /* child reads, doesn't write */
    close(ackPipe[ReadEnd]);

    buf = malloc(BytesToSend);
    ack[0] = 'C';
    ack[1] = '\n';
    ack[2] = '\0';

    for(int i = 0; i < NumberReps; i++) {
        read(bigPipe[ReadEnd], buf, BytesToSend);      /* read until end of byte stream */
        for(int j = 0; j < BytesToSend; j++) {
            ack[0] += buf[j];
        }
        ack[0] = 'A' + (ack[0] %26);
        write(ackPipe[WriteEnd], ack, AckLength);
    }
    write(STDOUT_FILENO, buf, BytesToSend);        /* echo to the standard output */

    close(bigPipe[ReadEnd]);                          /* close the ReadEnd: all done */
    close(ackPipe[WriteEnd]);
    _exit(0);                                         /* exit and notify parent at once  */
  }
  else {              /*** parent ***/
    close(bigPipe[ReadEnd]);                          /* parent writes, doesn't read */
    close(ackPipe[WriteEnd]);
    
    buf = malloc(BytesToSend);
    for(int i = 0; i < BytesToSend - 1; i++) {
        buf[i] = 'A' + (i % 26);
    }
    buf[BytesToSend-1] = '\n';
    
    for(int i = 0; i < NumberReps; i++) {
        OUTPUT_STATS(label);
        write(bigPipe[WriteEnd], buf, BytesToSend);       /* write the bytes to the pipe */
        read(ackPipe[ReadEnd], ack, AckLength);
        OUTPUT_STATS(label);
        write(STDOUT_FILENO, ack, AckLength);        /* echo to the standard output */
        buf[0]++;
    }
    close(bigPipe[WriteEnd]);                         /* done writing: generate eof */
    close(ackPipe[ReadEnd]);

    wait(NULL);                                       /* wait for child to exit */
    exit(0);                                          /* exit normally */
  }
  return 0;
}
