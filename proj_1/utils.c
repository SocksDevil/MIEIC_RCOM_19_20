
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "constants.h"

int stuff_buffer(unsigned char ** buffer, int length) {
    // determine new length
    int new_length = 0;
    for (int i = 0; i < length; i++) {
        new_length++;
        if ((*buffer)[i] == FLAG || (*buffer)[i] == ESCAPE_CHAR) new_length++;
    }

    // create new buffer
    unsigned char * new_buffer = (unsigned char*) malloc(new_length);
    memset(new_buffer, 0, sizeof(unsigned char) * new_length);

    // fill new buffer
    for (int i = 0, ins_pos = 0; i < length; i++) {
        if((*buffer)[i] == FLAG) {
            new_buffer[ins_pos] = ESCAPE_CHAR;
            new_buffer[ins_pos+1] = FLAG_SUBST;
            ins_pos++;
        }
        else if((*buffer)[i] == ESCAPE_CHAR) {
            new_buffer[ins_pos] = ESCAPE_CHAR;
            new_buffer[ins_pos+1] = ESCAPE_SUBST;
            ins_pos++;
        }
        else {
            new_buffer[ins_pos] = (*buffer)[i];
        }
        ins_pos++;
    }

    // assign new variables
    *buffer = new_buffer;
    return new_length;
}

int destuff_buffer(unsigned char ** buffer, int length) {
    // determine new_length
    int new_length = length;
    for (int i = 0 ; i < length; i++) {
        if ((*buffer)[i] == ESCAPE_CHAR) new_length--;
    }

    // create new buffer
    unsigned char * new_buffer = (unsigned char *) malloc(new_length);
    memset(new_buffer, 0, sizeof(unsigned char) * new_length);

    // fill new buffer
    for (int i = 0, ins_pos = 0; i < length; i++, ins_pos++) {        
        if ((*buffer)[i] == ESCAPE_CHAR) {
            if ((*buffer)[i+1] == FLAG_SUBST) {
                new_buffer[ins_pos] = FLAG;
            }
            else if ((*buffer)[i+1] == ESCAPE_SUBST) {
                new_buffer[ins_pos] = ESCAPE_CHAR;
            }
            i++;
        }
        else {
            new_buffer[ins_pos] = (*buffer)[i];
        }
    }

    // assign new variables
    *buffer = new_buffer;
    return new_length;
}

int parse_arguments(int argc, char *argv[]){
  if ((argc < 2) ||
      ((strcmp("0", argv[1]) != 0) &&
       (strcmp("1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial 1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  return strtol(argv[1], NULL, 10);
}

int byte_size(unsigned long num) {
    int size = 0;
    while(num != 0) {
        size++;
        num = num >> 8;
    }
    return size;
}

int file_size(char * filename) {

    FILE * fp = NULL;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    
    return ftell(fp);
}