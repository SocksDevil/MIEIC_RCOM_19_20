#include "serial_driver.h"
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
static struct termios oldtio;
static int send_cnt = 0;
static int fd;

void send_frame() {
  send_cnt++;
  printf("Sending message. Attempt %d\n", send_cnt);
  unsigned char SET[5];
  SET[0] = FLAG;
  SET[1] = A;
  SET[2] = C_SET;
  SET[3] = SET[1] ^ SET[2];
  SET[4] = FLAG;

  write(fd, SET, 5);

  // only try to send msg 3 times
  if (send_cnt == SEND_ATTEMPTS + 1) {
    printf("Could not send message. Exiting program.\n");
    exit(1);
  }

  alarm(TIMEOUT_T);
}

int open_connection(link_layer layer) {
  struct termios newtio;
  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  fd = open(layer.port, O_RDWR | O_NOCTTY);
  if (fd < 0) {
    perror(layer.port);
    exit(-1);
  }

  tcgetattr(fd, &oldtio); /* save current port settings */

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = layer.baud_rate | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;  /* blocking read until 1 chars received */

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &newtio);

  printf("Connection opened\n");
  return fd;
}

int close_connection(int fd){
  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
}

int update_state(int state, unsigned char  * received_frame){
  switch (state) {
    case STATE_FLAG_I:
      if (received_frame[state] == FLAG)
        state++;

      break;
    case STATE_A:

      if (received_frame[state] == A)
        state++;
      else if (received_frame[state] != FLAG)
        state = 0;

      break;

    case STATE_C:
      if (received_frame[state] == C_SET)
        state++;
      else if (received_frame[state] == FLAG)
        state = 1;
      else
        state = 0;
      break;

    case STATE_BCC:
      if (received_frame[state] == (C_SET ^ A))
        state++;
      else
        state = 0;
      break;
    case STATE_FLAG_E:
      if (received_frame[state] == FLAG) {
        state = STATE_END;
      }
      else
        state = STATE_FLAG_I;
  }
  return state;
}

void set_connection(){
  (void) signal(SIGALRM, send_frame);
  send_frame();
  alarm(TIMEOUT_T);

  unsigned char last = 0;
  int state = 0;
  char parity;
  unsigned char received_frame[5];
  while (state == STATE_END) {
    read(fd, &received_frame[state], 1);
    alarm(0);
    state = update_state(state, received_frame);
  }
}
