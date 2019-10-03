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

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07



volatile int STOP = FALSE;

int main(int argc, char **argv) {
  int fd, c, res;
  struct termios oldtio, newtio;
  char buf[1];

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
  bool success = false, error = false;
  while(!success && !error){
    read(fd, &received_set[state], 1);
    switch(state){
      case 0:
        if(received_set[state] == FLAG)
          state++;
        else 
          error = true;  

        break;
      case 1:

        if(received_set[state] == A)
          state++;
        else 
          error = true;  
        

        break;

      case 2:
        if(received_set[state]== C_SET)
          state++;
        else 
          error = true;  
        break;

      case 3:
        if(received_set[state]== (FLAG ^A) )
          state++;
        else 
          error = true;  
        break;
      case 4:
        if(received_set[state] == FLAG){
          success = true;
          state++;
        }
        else 
          error = true;  

    }
    

  }



  unsigned char sending_set[5];
  sending_set[0] = FLAG;
  sending_set[4] = FLAG;
  sending_set[1] = A;
  sending_set[2] = C_UA;
  sending_set[3] = sending_set[1] ^ sending_set[2];


  if(success){
    write(fd,sending_set,5);

  }
  
  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}
