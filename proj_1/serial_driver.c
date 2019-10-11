#include "serial_driver.h"
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>
#include "llinterpretation.h"

static struct termios oldtio;
static int send_cnt = 0;
static int fd;
static int timeout;

bool check_protection_field(
  frame_t *frame) {  
  return frame->received_frame[frame->current_frame] ==
         (frame->control_field ^ frame->received_frame[STATE_A]);
}

bool is_non_info_frame(unsigned char received_frame) {
  return received_frame == C_SET ||
         received_frame == C_DISC ||
         received_frame == C_UA ||
         received_frame == C_REJ_0 ||
         received_frame == C_REJ_1 ||
         received_frame == C_RR_0 ||
         received_frame == C_RR_1;
}

bool is_info_frame(frame_t *frame) {
  return frame->control_field == C_RI_0 ||
         frame->control_field == C_RI_1;
}

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

int close_connection(int fd) {
  if (tcsetattr(fd, TCSANOW, &oldtio)) {
    return close(fd);
  }
  return -1;
}

void update_bcc_state(frame_t *frame) {
  if (check_protection_field(frame)) {
    if (is_info_frame(frame)) {
      frame->current_state = STATE_DATA;
      return;
    }
    else if (is_non_info_frame(frame->received_frame[frame->current_frame])) {
      frame->current_state = STATE_FLAG_E;
      return;
    }
  }
  frame->current_state = STATE_ERROR;
}

void update_state(
  frame_t *frame) {
  switch (frame->current_state) {
    case STATE_FLAG_I:
      if (frame->received_frame[frame->current_frame] == FLAG)
        frame->current_state = STATE_A;

      break;
    case STATE_A:

      if (frame->received_frame[frame->current_frame] == A){
        frame->current_state = STATE_C;
        break;
      }
      else if (frame->received_frame[frame->current_frame] != FLAG){
        frame->current_state = STATE_FLAG_I;
        break;
      }
      frame->current_frame--;

      break;

    case STATE_C:
      if (frame->received_frame[frame->current_frame] == frame->control_field){
        frame->current_state = STATE_BCC;
      }
      else if (frame->received_frame[frame->current_frame] == FLAG){
        frame->current_state = STATE_A;
        frame->current_frame--;
      }
      else{
        frame->current_state = STATE_FLAG_I;
        frame->current_frame = -1;
      }
      break;

    case STATE_BCC:
      update_bcc_state(frame);
      break;

    case STATE_DATA:
      if (frame->received_frame[frame->current_frame] == FLAG)
        frame->current_state = STATE_END;

      break;

    case STATE_FLAG_E:
      if (frame->received_frame[frame->current_frame] == FLAG)
        frame->current_state = STATE_END;
      else{
        frame->current_state = STATE_FLAG_I;
        frame->current_frame = -1;
      }
    default:
      break;
  }
}

void set_connection(link_layer layer) {
  timeout = layer.timeout;
  (void) signal(SIGALRM, send_frame);
  send_frame();
  alarm(timeout);
  unsigned char received_frame[5];
  frame_t frame = {
    .current_state = STATE_FLAG_I,
    .current_frame = 0,
    .received_frame = received_frame,
    .control_field = C_UA};
  for (;
       frame.current_frame < MAX_SIZE && frame.current_state != STATE_END && frame.current_state != STATE_ERROR;
       frame.current_frame++) {
    read(fd, &received_frame[frame.current_frame], 1);
    alarm(0);
    update_state(&frame);
  }
}

void acknowledge_connection() {

  unsigned char received_frame[5];
  frame_t frame = {
    .current_state = STATE_FLAG_I,
    .current_frame = 0,
    .received_frame = received_frame,
    .control_field = C_SET};
  for (;
       frame.current_frame < MAX_SIZE && frame.current_state != STATE_END && frame.current_state != STATE_ERROR;
       frame.current_frame++) {
    read(fd, &received_frame[frame.current_frame], 1);
    update_state(&frame);
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

void send_non_info_frame(unsigned char control_field) {
  char sending_ack[5];
  sending_ack[0] = FLAG;
  sending_ack[4] = FLAG;
  sending_ack[1] = A;
  sending_ack[2] = control_field;
  sending_ack[3] = sending_ack[1] ^ sending_ack[2];

  write(fd, sending_ack, 5);
}

int read_data(int fd, int sequence_number, char *buffer) {
  unsigned char received_frame[MAX_SIZE];

  frame_t frame = {
    .current_state = STATE_FLAG_I,
    .current_frame = 0,
    .received_frame = received_frame,
    .control_field = sequence_number == 0 ? C_RI_0 : C_RI_1,
  };

  for (;
       frame.current_frame < MAX_SIZE && frame.current_state != STATE_END && frame.current_state != STATE_ERROR;
       frame.current_frame++) {
    read(fd, &frame.received_frame[frame.current_frame], 1);
    update_state(&frame);
  }

  int data_size;

  if (frame.current_state != STATE_ERROR &&
      (data_size = interpreter(frame.received_frame, buffer)) != -1) {
    printf("Receiver ready!\n");    
    send_non_info_frame(sequence_number == 0 ? C_RR_0 : C_RR_1);
  }
  else {
    printf("Package rejected!\n");
    send_non_info_frame(sequence_number == 0 ? C_REJ_0 : C_REJ_0);
    return -1;
  }

  return data_size;
}

int write_data(int fd, int sequence_number, char *buffer, int length) {
  if (length + 6 <= MAX_SIZE) {
    unsigned char data_frame[MAX_SIZE];
    int index = 4;
    data_frame[0] = FLAG;
    data_frame[1] = A;
    data_frame[2] = sequence_number == 0 ? C_RI_0 : C_RI_1;
    data_frame[3] = data_frame[1] ^ data_frame[2];
    char xor = buffer[0];
    data_frame[index++] = buffer[0];
    for (int i = 1; i < length; index++, i++) { //Generalize with llinterpretation
      xor ^= buffer[i];
      data_frame[index] = buffer[i];
    }
    data_frame[index++] = xor;
    data_frame[index++] = FLAG;
    int written_bytes = write(fd, data_frame, index);
    unsigned char received_frame[MAX_SIZE];

    frame_t frame = {
      .current_state = STATE_FLAG_I,
      .current_frame = 0,
      .received_frame = received_frame,
      .control_field = sequence_number == 0 ? C_RR_0 : C_RR_1,
    };

    for (;
         frame.current_frame < MAX_SIZE && frame.current_state != STATE_END && frame.current_state != STATE_ERROR;
         frame.current_frame++) {
      read(fd, &frame.received_frame[frame.current_frame], 1);
      update_state(&frame);
    }
    if (frame.current_state == STATE_ERROR)
      return -1;
    return written_bytes;
  }
  return -1;
}