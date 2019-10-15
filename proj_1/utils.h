#pragma once

#include <stdint.h>

int stuff_buffer(uint8_t **buffer, int length);

int destuff_buffer(uint8_t **buffer, int length);

int parse_arguments(int argc, char *argv[]);

unsigned char byte_size(long num);

long file_size(char *filename);

int frame_cmp(unsigned char *p1, unsigned char *p2);