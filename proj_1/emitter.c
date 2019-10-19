#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ll.h"
#include "utils.h"
#include "app_layer.h"

int main(int argc, char * argv[]) {
    int port = parse_arguments(argc, argv);

    int fd = llopen(port, TRANSMITTER);
    if(fd == -1){
        printf("Something went wrong!\n");
        return -1;
    }

    if (send_file(argv[2], fd) == -1) {
        printf("Error sending file %s\n", argv[2]);
        return -1;
    }
    
    printf("Written file %s to serial port\n", argv[2]);
    
    llclose(fd, TRANSMITTER);
    return 0;
}
