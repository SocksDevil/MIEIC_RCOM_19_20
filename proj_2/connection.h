#pragma once

#include "hostname.h"

#define MAX_SIZE 2500

#define TCP_READY "220 " // extra space is important
#define TCP_NEED_PWD "331"
#define TCP_LOGIN_SUCCESS "230"
#define TCP_CLOSE "221"

// Read info in the socket until it matches with match
int recvuntil(int fd, const char *match);

// Read all available info in the socket
int recvall(int fd);

int ftp_login(int socketfd, url_info_t url_info);

int ftp_disconnect(int socketfd);