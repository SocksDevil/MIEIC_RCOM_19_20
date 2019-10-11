#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ll.h"
#include "utils.h"

int main(int argc, char * argv[]) {
    int port = parse_arguments(argc, argv);

    int fd = llopen(port, TRANSMITTER);
    if(fd == -1){
        printf("Something went wrong!\n");
        return -1;
    }
    int i = 0;
    for (; argv[2][i] != '\0'; i++) {}
    llwrite(fd, argv[2], ++i);
    printf("Written %s to serial port\n", argv[2]);

    printf("Sucess!\n");
    
    llclose(fd);
    return 0;
}
