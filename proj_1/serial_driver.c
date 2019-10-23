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
#include "disconnect_frame.h"
#include "llinterpretation.h"
#include "utils.h"

static struct termios oldtio;
static int send_cnt = 0;
static int fd;
static int timeout;

bool check_protection_field(
  frame_t *frame) {
  return frame->received_frame[STATE_BCC] ==
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

void send_set_up_frame() {
  send_cnt++;
  unsigned char SET[5];
  SET[0] = FLAG;
  SET[1] = EMITTER_A;
  SET[2] = C_SET;
  SET[3] = SET[1] ^ SET[2];
  SET[4] = FLAG;

  write(fd, SET, 5);

  // only try to send msg SEND_ATTEMPTS times
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

void check_data_frame(frame_t *frame) {
  if (frame->received_frame[frame->current_frame] ==
      (frame->sequence_number == 0 ? C_REJ_0 : C_REJ_1)) {
    frame->current_state = STATE_ERROR;
  }
  else {
    check_control_field(frame);
  }
}

void read_data_frame(frame_t *frame) {
  if (frame->received_frame[frame->current_frame] ==
      (frame->sequence_number == 0 ? C_RI_1 : C_RI_0)) {
    frame->current_state = STATE_WRONG_SEQ_NUM;
  }
  else {
    check_control_field(frame);
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
      if (frame->received_frame[frame->current_frame] == FLAG) {
        frame->current_state = STATE_A;
        break;
      }
      frame->current_frame--;
      break;
    case STATE_A:

      if (frame->received_frame[frame->current_frame] == frame->address_field) {
        frame->current_state = STATE_C;
        break;
      }
      else if (frame->received_frame[frame->current_frame] != FLAG) {
        frame->current_state = STATE_FLAG_I;
        frame->current_frame = STATE_FLAG_I;
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

    case STATE_WRONG_SEQ_NUM:
      if (frame->received_frame[frame->current_frame] == FLAG)
        frame->current_state = STATE_WRONG_SEQ_END;
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
      break;
    case STATE_DISC:
      if (frame->received_frame[frame->current_frame] == FLAG) {
        if (check_protection_field(frame)) {
          frame->current_state = STATE_DISC_END;
        }
        else {
          frame->current_state = STATE_ERROR;
        }
      }
      break;
    default:
      break;
  }
}

frame_t read_control_frame(unsigned char control_field, unsigned char address_field) {
  unsigned char received_frame[5];
  frame_t frame = {
    .current_state = STATE_FLAG_I,
    .current_frame = 0,
    .received_frame = received_frame,
    .control_field = control_field,
    .address_field = address_field,
    .control_verification = check_control_field};

  for (;
       frame.current_frame < MAX_SIZE && frame.current_state != STATE_END && frame.current_state != STATE_ERROR;
       frame.current_frame++) {
    read(fd, &received_frame[frame.current_frame], 1);
    alarm(0); // TODO- Rethink how alarm reads could be approached
    update_state(&frame);
  }
  return frame;
}

int close_connection(int fd) {
  if (tcsetattr(fd, TCSANOW, &oldtio) != 0) {
    printf("Failed to set back the old termio!\n");
    return -1;
  }
  return close(fd);
}

int emitter_disconnect(int fd) {
  //send disconnect
  //wait for disconnect
  //send UA

  launch_disconnect_alarm(fd, 3, EMITTER_A);

  frame_t frame = read_control_frame(C_DISC, RECEPTOR_A);

  if (frame.current_state != STATE_END) {
    return -1;
  }
  printf("Disconnecting\n");
  send_non_info_frame(fd, C_UA, RECEPTOR_A);
  return close_connection(fd);
}

int set_connection(link_layer layer) {
  timeout = layer.timeout;
  set_timeout(timeout);
  (void) signal(SIGALRM, send_set_up_frame);
  send_set_up_frame();
  alarm(timeout);
  frame_t frame = read_control_frame(C_UA, EMITTER_A);
  if (frame.current_state == STATE_END)
    return 0;
  return -1;
}

int acknowledge_connection() {

  frame_t frame = read_control_frame(C_SET, EMITTER_A);

  if (frame.current_state == STATE_END) {
    unsigned char sending_set[5];
    sending_set[0] = FLAG;
    sending_set[4] = FLAG;
    sending_set[1] = EMITTER_A;
    sending_set[2] = C_UA;
    sending_set[3] = sending_set[1] ^ sending_set[2];

    write(fd, sending_set, 5);
    return 0;
  }
  return -1;
}

int receptor_disconnect(int fd) {
  frame_t disc_frame = read_control_frame(C_DISC, EMITTER_A);
  if (disc_frame.current_state != STATE_END) {
    printf("Received wrong disconnect frame\n");
    return -1;
  }
  printf("Disconnecting\n");
  return receptor_send_disconnect(fd);
}

int receptor_send_disconnect(int fd) {
  launch_disconnect_alarm(fd, 3, RECEPTOR_A);
  frame_t ua_frame = read_control_frame(C_UA, RECEPTOR_A);

  if (ua_frame.current_state != STATE_END) {
    printf("Received wrong UA disconnect frame\n");
    return -1;
  }

  return close_connection(fd);
}

int read_data(int fd, int sequence_number, char *buffer) {
  unsigned char received_frame[MAX_SIZE];

  frame_t frame = {
    .current_state = STATE_FLAG_I,
    .current_frame = 0,
    .received_frame = received_frame,
    .control_field = sequence_number == 0 ? C_RI_0 : C_RI_1,
    .sequence_number = sequence_number,
    .address_field = EMITTER_A,
    .control_verification = check_data_frame};

  for (;
       frame.current_frame < MAX_SIZE &&
       frame.current_state != STATE_END &&
       frame.current_state != STATE_ERROR &&
       frame.current_state != STATE_WRONG_SEQ_END &&
       frame.current_state != STATE_DISC_END;
       frame.current_frame++) {
    read(fd, &frame.received_frame[frame.current_frame], 1);
    // printf("Current value: %2x, current state: %d\n", frame.received_frame[frame.current_frame], frame.current_state);
    update_state(&frame);
  }

  int data_size = destuff_buffer(frame.received_frame, frame.current_frame);

  if (frame.current_state != STATE_ERROR &&
      (data_size = interpreter(frame.received_frame, buffer, data_size)) != -1) {
    send_non_info_frame(fd, sequence_number == 0 ? C_RR_0 : C_RR_1, EMITTER_A);
    save_last_frame(received_frame, sequence_number);
  }
  else if (frame.current_state == STATE_ERROR) {
    printf("Package rejected!\n");
    send_non_info_frame(fd, sequence_number == 0 ? C_REJ_0 : C_REJ_1, EMITTER_A);
    return -1;
  }
  else if (frame.current_state == STATE_WRONG_SEQ_END) {
    int opposite_seq_num = (sequence_number ^ 1);
    if (is_same_frame(received_frame, opposite_seq_num))
      send_non_info_frame(fd, opposite_seq_num == 0 ? C_RR_0 : C_RR_1, EMITTER_A);
    else
      send_non_info_frame(fd, opposite_seq_num == 0 ? C_REJ_0 : C_REJ_1, EMITTER_A);
    return -1;
  }
  else if (frame.current_state == STATE_DISC) {
    return DISC_ON_READ;
  }
  return data_size;
}

int write_data(int fd, int sequence_number, char *buffer, int length) {
  int written_bytes = prepare_data_frame(sequence_number, buffer, length, fd);
  if (written_bytes == -1)
    return -1;

  (void) signal(SIGALRM, write_data_frame);
  write_data_frame();
  unsigned char received_frame[MAX_SIZE];

  frame_t frame = {
    .current_state = STATE_FLAG_I,
    .current_frame = 0,
    .received_frame = received_frame,
    .control_field = sequence_number == 0 ? C_RR_0 : C_RR_1,
    .sequence_number = sequence_number,
    .address_field = EMITTER_A,
    .control_verification = check_data_frame};
  for (;
       frame.current_frame < MAX_SIZE && frame.current_state != STATE_END && frame.current_state != STATE_ERROR;
       frame.current_frame++) {
    read(fd, &frame.received_frame[frame.current_frame], 1);
    update_state(&frame);
  }

  alarm(0);
  if (frame.current_state != STATE_END) {
    printf("Receiver rejected!\n");
    return -1;
  }
  return written_bytes;
}
