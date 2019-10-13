#pragma once

int prepare_data_frame(int sequence_number, char *buffer, int length, int file_descriptor);

void write_data_frame();

void set_timeout(int new_timeout);