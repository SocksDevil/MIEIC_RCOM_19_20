#pragma once

#include <stdint.h>

int stuff_buffer(uint8_t ** buffer, int length);

int destuff_buffer(uint8_t ** buffer, int length);
 
int parse_arguments(int argc, char *argv[]);

unsigned char byte_size(long num);

long file_size(char * filename);

void send_non_info_frame(int fd, unsigned char control_field);