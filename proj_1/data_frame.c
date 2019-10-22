#include "data_frame.h"
#include "constants.h"
#include "utils.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char data_frame0[MAX_SIZE];
static unsigned char data_frame1[MAX_SIZE];
static int current_sequence_number = 0;
static int fd = -1;
static int frame_size = 0;
static int num_attempts = 4;
static int current_attempt = 0;
static int timeout = 0;

int prepare_data_frame(int sequence_number, char *buffer, int length, int file_descriptor) {
  unsigned char *data_frame = sequence_number == 0 ? data_frame0 : data_frame1;
  current_attempt = 0;

  if (length + 6 <= MAX_SIZE) {
    //unsigned char data_frame[MAX_SIZE];
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
    current_sequence_number = sequence_number;
    fd = file_descriptor;

    frame_size = stuff_buffer(data_frame, index);

    return frame_size;
  }
  return -1;
}

void set_timeout(int new_timeout){
    timeout = new_timeout;
}

void write_data_frame(){
    if(current_attempt < num_attempts){
        if(fd != -1){
            current_attempt++;
            printf("Sending data frame: attempt %d\n", current_attempt);

            write(fd, current_sequence_number == 0 ? data_frame0 : data_frame1, frame_size);
            alarm(timeout);
        }
    } else {
        printf("Failed to send data frame %d times! Exiting...\n", num_attempts);
        exit(1);
    }

}

void save_last_frame(unsigned char * received_frame, int sequence_number){
  unsigned char * data_frame = sequence_number == 0 ? data_frame0 : data_frame1;
  frame_cpy(data_frame, received_frame);
}

bool is_same_frame(unsigned char * received_frame, int sequence_number){
  unsigned char *data_frame = sequence_number == 0 ? data_frame0 : data_frame1;
  return frame_cmp(received_frame, data_frame) == 0;
}
