#include "connection.h"

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
    sprintf(buf, "user %s\r\n", url_info.user);
    printf("> %s", buf);
    if (write(socketfd, buf, strlen(buf)) == -1) {
        perror("write user");
        return -1;
    }

    /* Password */
    recvuntil(socketfd, TCP_NEED_PWD);
    sprintf(buf, "pass %s\r\n", url_info.password);
    printf("> %s", buf);
    if (write(socketfd, buf, strlen(buf)) == -1) {
        perror("write password");
        return -1;
    }

    /* Wait for login */
    recvuntil(socketfd, TCP_LOGIN_SUCCESS);

    return 0;
}

/*
Info like:
227 Entering Passive Mode (193,137,29,15,233,220). 
*/
int parse_pasv_return(char * buff, pasv_info_t * pasv_info) {
    
    //char ip1[4], ip2[4], ip3[4], ip4[4];
    int ip1, ip2, ip3, ip4, portHigh, portLow;
    if (sscanf(buff, "%*99[^(](%4d,%4d,%4d,%4d,%10d,%10d).", &ip1, &ip2, &ip3, &ip4, &portHigh, &portLow) != 6) {
        printf("parse pasv sscanf did not match 6 values\n");
        return -1;
    }

    sprintf(pasv_info->ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    pasv_info->port = (portHigh << 8)  + portLow;

    return 0;
}

int ftp_passive_mode(int socketfd, pasv_info_t * pasv_info) {

    char buff[MAX_SIZE];

    /* Write pasv command */
    sprintf(buff, "pasv\r\n");
    printf("> %s", buff);
    if (write(socketfd, buff, strlen(buff)) == -1) {
        perror("write user");
        return -1;
    }

    /* Read pasv command return */
    memset(buff, 0, MAX_SIZE*sizeof(char));
    if (read(socketfd, buff, MAX_SIZE) == -1) {
        perror("read pasv return");
        return -1;
    }

    if (strstr(buff, TCP_PASV) == NULL) {
        printf("wrong pasv return\n");
        return -1;
    }
    printf("%s", buff);

    parse_pasv_return(buff, pasv_info); 

    return 0;
}



int ftp_disconnect(int socketfd) {
    char buf[MAX_SIZE];
    sprintf(buf, "quit\r\n");
    printf("> %s", buf);
    if (write(socketfd, buf, strlen(buf)) == -1) {
        perror("write user");
        return -1;
    }
    recvuntil(socketfd, TCP_CLOSE);

    return 0;
}