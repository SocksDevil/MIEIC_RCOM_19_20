#pragma once

int stuff_buffer(unsigned char ** buffer, int length);

int destuff_buffer(unsigned char ** buffer, int length);
 
int parse_arguments(int argc, char *argv[]);

int byte_size(unsigned long num);

int file_size(char * filename);