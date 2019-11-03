
#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"

int stuff_buffer(unsigned char * buffer, int length) {
    // determine new length
    int new_length = 5;
    for (int i = 4; i < length-1; i++) {
        new_length++;
        if (buffer[i] == FLAG || buffer[i] == ESCAPE_CHAR) new_length++;
    }

    // create new buffer
    unsigned char * new_buffer = (unsigned char*) malloc(new_length);
    memset(new_buffer, 0, sizeof(unsigned char) * new_length);

    // copy header and trailing info
    memcpy(new_buffer, buffer, sizeof(unsigned char) * 4);
    new_buffer[new_length-1] = buffer[length-1];

    // fill new buffer
    for (int i = 4, ins_pos = 4; i < length-1; i++) {
        if(buffer[i] == FLAG) {
            new_buffer[ins_pos] = ESCAPE_CHAR;
            new_buffer[ins_pos+1] = FLAG_SUBST;
            ins_pos++;
        }
        else if(buffer[i] == ESCAPE_CHAR) {
            new_buffer[ins_pos] = ESCAPE_CHAR;
            new_buffer[ins_pos+1] = ESCAPE_SUBST;
            ins_pos++;
        }
        else {
            new_buffer[ins_pos] = buffer[i];
        }
        ins_pos++;
    }

    memcpy(buffer, new_buffer, new_length);
    // assign new variables
    return new_length;
}

int destuff_buffer(unsigned char * buffer, int length) {
    // determine new_length
    int new_length = length;
    for (int i = 0 ; i < length; i++) {
        if (buffer[i] == ESCAPE_CHAR) new_length--;
    }

    // create new buffer
    unsigned char new_buffer[new_length];
    memset(new_buffer, 0, sizeof(unsigned char) * new_length);

    // copy header and trailing info
    memcpy(new_buffer, buffer, sizeof(unsigned char) * 4);
    new_buffer[new_length-1] = buffer[length-1];

    // fill new buffer
    for (int i = 4, ins_pos = 4; i < length-1; i++, ins_pos++) {        
        if (buffer[i] == ESCAPE_CHAR) {
            if (buffer[i+1] == FLAG_SUBST) {
                new_buffer[ins_pos] = FLAG;
            }
            else if (buffer[i+1] == ESCAPE_SUBST) {
                new_buffer[ins_pos] = ESCAPE_CHAR;
            }
            i++;
        }
        else {
            new_buffer[ins_pos] = buffer[i];
        }
    }

    memcpy(buffer, new_buffer, new_length);

    // assign new variables
    return new_length;
}

void send_non_info_frame(int fd, unsigned char control_field, unsigned char address_field) {
  char sending_ack[5];
  sending_ack[0] = FLAG;
  sending_ack[4] = FLAG;
  sending_ack[1] = address_field;
  sending_ack[2] = control_field;
  sending_ack[3] = sending_ack[1] ^ sending_ack[2];

  write(fd, sending_ack, 5);
}

bool check_null(int argc, char* argv[]){
  return (argc >= 3) &&(
         ((argc == 4) &&
          ((strcmp("0", argv[1]) == 0) ||
          (strcmp("1", argv[1]) == 0)) &&
          (strcmp("emitter", argv[2]) == 0)) ||
         ((argc == 3) &&
          ((strcmp("0", argv[1]) == 0) ||
          (strcmp("1", argv[1]) == 0)) &&
          (strcmp("receiver", argv[2]) == 0)));
}

connection_type parse_arguments(int argc, char *argv[]) {
  if (!check_null(argc, argv)){
      printf("Usage: main <serial port number> emitter/receiver (filename)\n");
      exit(1);
  }

  connection_type conn_type = {
      .role = argc == 3 ? RECEIVER: TRANSMITTER,
      .filename = argv[3],
      .port_num = strtol(argv[1], NULL, 10)
    };


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  return conn_type;
}

unsigned char byte_size(long num) {
    unsigned char size = 0;
    while(num != 0) {
        size++;
        num = num >> 8;
    }
    return size;
}

long file_size(char * filename) {

    FILE * fp = NULL;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    
    return ftell(fp);
}

int frame_cmp(unsigned char *p1, unsigned char *p2) {
      unsigned char * s1 = p1;
      unsigned char * s2 = p2;
      unsigned char c1,c2;
    if(*s1 != *s2)
        return s1-s2;

  do{
    s1++;
    s2++;
      c1 = *s1;
      c2 = *s2;
      if(c1 == FLAG)
        return c1-c2;

  }while(c1 == c2);
    return c1-c2;


}

unsigned char* frame_cpy(unsigned char *dest,unsigned char *src){
    *dest = *src;

    do{
        src++;
        dest++;
        *dest = *src;
    }
    while(*src != FLAG);
    
    return dest;
}

bool random_failure(int failure_rate){
  int random = rand();
  return random < (RAND_MAX * failure_rate);
}