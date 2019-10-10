#pragma once
#include "constants.h"

int open_connection(link_layer layer);

void set_connection(link_layer layer);

void acknowledge_connection();

int close_connection(int fd);