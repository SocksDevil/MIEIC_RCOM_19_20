/*Non-Canonical Input Processing*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "constants.h"


int main(int argc, char **argv) {
  int fd;
  struct termios oldtio, newtio;

  if ((argc < 2) ||
      ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
       (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0) {
    perror(argv[1]);
    exit(-1);
  }

  tcgetattr(fd, &oldtio); /* save current port settings */

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;  /* blocking read until 1 chars received */

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &newtio);

  printf("New termios structure set\n");

  unsigned char received_set[5];

  int state = 0;
  bool success = false;
  while(!success ){
    read(fd, &received_set[state], 1);
    switch(state){
      case STATE_FLAG_I:
        if(received_set[state] == FLAG)
          state++;
         

        break;
      case STATE_A:

        if(received_set[state] == A)
          state++;
        else 
          if(received_set[state] != FLAG)
            state = 0; 
        

        break;

      case STATE_C:
        if(received_set[state]== C_SET)
          state++;
        else 
          if(received_set[state]== FLAG)
            state = 1;
          else
            state = 0;
        break;

      case STATE_BCC:
        if(received_set[state]== (C_SET ^A) )
          state++;
        else 
          state = 0; 
        break;
      case STATE_FLAG_E:
        if(received_set[state] == FLAG){
          success = true;
          state++;
        }
        else 
          state = STATE_FLAG_I;

    }

    printf("Current state: %d\n", state);
  }





  if(success){
    unsigned char sending_set[5];
    sending_set[0] = FLAG;
    sending_set[4] = FLAG;
    sending_set[1] = A;
    sending_set[2] = C_UA;
    sending_set[3] = sending_set[1] ^ sending_set[2];
    
    printf("Success, sending message\n");
    write(fd,sending_set,5);

  }
  
  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}
