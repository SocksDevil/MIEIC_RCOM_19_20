#pragma once

#include "hostname.h"

#define MAX_SIZE 2500

// Read info in the socket until it matches with match
int recvuntil(int fd, const char *match);

// Read all available info in the socket
int recvall(int fd);

int ftp_login(int socketfd, url_info_t url_info);
