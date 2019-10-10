#pragma once
#include "constants.h"

int llopen(int port, connection_role role);

int llwrite(int fd, char * buffer, int length);

int llclose(int fd);