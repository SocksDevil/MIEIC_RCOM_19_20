#pragma once

#include <stdint.h>

int stuff_buffer(uint8_t ** buffer, int length);

int destuff_buffer(uint8_t ** buffer, int length);
 
int parse_arguments(int argc, char *argv[]);

int byte_size(unsigned long num);

int file_size(char * filename);