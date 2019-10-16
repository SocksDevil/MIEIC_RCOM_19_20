#pragma once
#include "constants.h"

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

int llopen(int port, connection_role role);

int llclose(int fd, connection_role role);