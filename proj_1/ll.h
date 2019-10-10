#include "constants.h"
int llopen(int fd, connection_role role);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

int llclose(int fd);