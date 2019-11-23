#include "statemachine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

int recvuntil(int fd, const char * match) {
    char buff[MAX_SIZE];
    memset(buff, 0, MAX_SIZE*sizeof(char));
    
    char * ret;
    int read_ret;
    while((ret = strstr(buff, match)) == NULL) {

        memset(buff, 0, MAX_SIZE * sizeof(char));

        if ((read_ret = read(fd, buff, MAX_SIZE)) == -1) {
            perror("Read from socket\n");
            return -1;
        }

        if (read_ret == 0) {
            printf("Could not find match in recvuntil\n");
            return -1;
        }

        // Print string read from socket
        printf("%s", buff);
    }

    return 0;
}

int recvall(int fd) {
    char buff[MAX_SIZE];
    memset(buff, 0, MAX_SIZE*sizeof(char));
    
    int read_ret;
    while((read_ret = recv(fd, buff, MAX_SIZE, MSG_DONTWAIT)) != -1) {

        // Print string read from socket
        printf("%s", buff);

        // Clear buffer
        memset(buff, 0, MAX_SIZE * sizeof(char));
    }
    
    // Verify if exited normally
    if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
    return -1;
}

int ftp_login(int socketfd, url_info_t url_info) {

    char buf[MAX_SIZE];

    /* User */
    recvuntil(socketfd, "220 ");
    sprintf(buf, "user %s\r\n", url_info.user);
    printf("> %s\n", buf);
    if (write(socketfd, buf, strlen(buf)) == -1) {
        perror("write user");
        return -1;
    }

    /* Password */
    recvuntil(socketfd, "password");
    sprintf(buf, "pass %s\r\n", url_info.password);
    printf("> %s\n", buf);
    if (write(socketfd, buf, strlen(buf)) == -1) {
        perror("write password");
        return -1;
    }

    /* Wait for login */
    recvuntil(socketfd, "successful");

    return 0;
}