#pragma once
#include <stdbool.h>

int prepare_data_frame(int sequence_number, char *buffer, int length, int file_descriptor);

void write_data_frame();

void set_timeout(int new_timeout);

bool is_same_frame(unsigned char *received_frame, int sequence_number);

void save_last_frame(unsigned char *received_frame, int sequence_number);
