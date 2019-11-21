#include "statemachine.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

    printf("\n");

    return 0;
}