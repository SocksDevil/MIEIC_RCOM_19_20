#include "serial_driver.h"
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <strings.h>
static struct termios oldtio;
static int send_cnt = 0;
static int fd;
static int timeout;

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

  alarm(timeout);
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
  if (tcsetattr(fd, TCSANOW, &oldtio)){
    return close(fd);
  }
  return -1;
}

int update_state(state_machine state, unsigned char  * received_frame, unsigned char control_camp){
  switch (state) {
    case STATE_FLAG_I:
      if (received_frame[state] == FLAG)
        state = STATE_A;

      break;
    case STATE_A:

      if (received_frame[state] == A)
        state = STATE_C;
      else if (received_frame[state] != FLAG)
        state = STATE_FLAG_I;

      break;

    case STATE_C:
      if (received_frame[state] == control_camp)
        state = STATE_BCC;
      else if (received_frame[state] == FLAG)
        state = STATE_A;
      else
        state = STATE_FLAG_I;
      break;

    case STATE_BCC:
      if (received_frame[state] == (control_camp ^ A))
        state = STATE_FLAG_E;
      else
        state = STATE_FLAG_I;
      break;
    case STATE_FLAG_E:
      if (received_frame[state] == FLAG) {
        state = STATE_END;
      }
      else
        state = STATE_FLAG_I;
    case STATE_END:
        break;    
  }
  return state;
}

void set_connection(link_layer layer){
  timeout = layer.timeout;
  (void) signal(SIGALRM, send_frame);
  send_frame();
  alarm(timeout);

  state_machine state = STATE_FLAG_I;
  unsigned char received_frame[5];
  while (state != STATE_END) {
    read(fd, &received_frame[state], 1);
    alarm(0);
    state = update_state(state, received_frame, C_UA);
  }
}

void acknowledge_connection(){

  state_machine state = STATE_FLAG_I;
  unsigned char received_frame[5];
  while (state == STATE_END) {
    read(fd, &received_frame[state], 1);
    alarm(0);
    state = update_state(state, received_frame, C_SET);
  }

  unsigned char sending_set[5];
  sending_set[0] = FLAG;
  sending_set[4] = FLAG;
  sending_set[1] = A;
  sending_set[2] = C_UA;
  sending_set[3] = sending_set[1] ^ sending_set[2];

  printf("Success, sending UA\n");
  write(fd, sending_set, 5);
}
