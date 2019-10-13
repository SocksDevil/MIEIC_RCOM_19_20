#include "serial_driver.h"

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#include "data_frame.h"
#include "llinterpretation.h"

static struct termios oldtio;
static int send_cnt = 0;
static int fd;
static int timeout;

static bool to_resend = false;

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

void check_control_field(frame_t *frame) {
  if (frame->received_frame[frame->current_frame] == frame->control_field) {
    frame->current_state = STATE_BCC;
  }
  else if (frame->received_frame[frame->current_frame] == FLAG) {
    frame->current_state = STATE_A;
    frame->current_frame--;
  }
  else {
    frame->current_state = STATE_FLAG_I;
    frame->current_frame = -1;
  }
}

void check_data_ack(frame_t *frame) {
  if (frame->received_frame[frame->current_frame] != 
      (frame->sequence_number == 0 ? C_REJ_0 : C_REJ_1) ) {
    check_control_field(frame);
  }else{
    frame->current_state = STATE_ERROR;
  }
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
    else if (is_non_info_frame(frame->received_frame[STATE_C])) {
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

      if (frame->received_frame[frame->current_frame] == A) {
        frame->current_state = STATE_C;
        break;
      }
      else if (frame->received_frame[frame->current_frame] != FLAG) {
        frame->current_state = STATE_FLAG_I;
        break;
      }
      frame->current_frame--;

      break;

    case STATE_C:
      frame->control_verification(frame);
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
      else {
        frame->current_state = STATE_FLAG_I;
        frame->current_frame = -1;
      }
    default:
      break;
  }
}

void set_connection(link_layer layer) {
  timeout = layer.timeout;
  set_timeout(timeout);
  (void) signal(SIGALRM, send_frame);
  send_frame();
  alarm(timeout);
  unsigned char received_frame[5];
  frame_t frame = {
    .current_state = STATE_FLAG_I,
    .current_frame = 0,
    .received_frame = received_frame,
    .control_field = C_UA,
    .control_verification = check_control_field};
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
    .control_field = C_SET,
    .control_verification = check_control_field};
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
    .sequence_number = sequence_number,
    .control_verification = check_control_field};

  for (;
       frame.current_frame < MAX_SIZE && frame.current_state != STATE_END && frame.current_state != STATE_ERROR;
       frame.current_frame++) {
    read(fd, &frame.received_frame[frame.current_frame], 1);
    update_state(&frame);
  }
  printf("\n");

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
  int written_bytes = prepare_data_frame(sequence_number, buffer, length, fd);
  if (written_bytes == -1)
    return -1;
  (void) signal(SIGALRM, write_data_frame);
  do {
    write_data_frame();
    unsigned char received_frame[MAX_SIZE];

    frame_t frame = {
      .current_state = STATE_FLAG_I,
      .current_frame = 0,
      .received_frame = received_frame,
      .control_field = sequence_number == 0 ? C_RR_0 : C_RR_1,
      .sequence_number = sequence_number,
      .control_verification = check_data_ack};
    for (;
         frame.current_frame < MAX_SIZE && frame.current_state != STATE_END && frame.current_state != STATE_ERROR;
         frame.current_frame++) {
      read(fd, &frame.received_frame[frame.current_frame], 1);
      update_state(&frame);
    }
    printf("%d\n", frame.current_state);
    if (frame.current_state == STATE_END) {
      printf("Received receiver ready!\n");
      alarm(0);
      to_resend = false;
    }
    else {
      printf("Receiver rejected!\n");
      to_resend = true;
    }
  } while (to_resend);

  return written_bytes;
}