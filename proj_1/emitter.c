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

    printf("Sucess!\n");
    
    llclose(fd);
    return 0;
}
