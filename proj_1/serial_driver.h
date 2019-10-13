#pragma once
#include <stdbool.h>

#include "constants.h"

typedef struct Frame frame_t;

typedef void (*control_func)(frame_t *);

int open_connection(link_layer layer);

void set_connection(link_layer layer);

void acknowledge_connection();

int close_connection(int fd);

int read_data(int fd, int sequence_number, char *buffer);

int write_data(int fd, int sequence_number, char *buffer, int length);

struct Frame {
  state_machine current_state;
  int current_frame;
  unsigned char control_field;
  unsigned char *received_frame;
  bool sequence_number;
  control_func control_verification;
};

