#include "set_frame.h"
#include "constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static int current_attempt = 0;
static int timeout = 0;
static int fd = -1;
static int max_attempts = 0;
static unsigned char SET[5];

void write_set(){
    if(current_attempt < max_attempts){
        if(fd != -1){
            current_attempt++;
            printf("Sending set control frame. Attempt: %d\n",current_attempt);
            write(fd,SET,5);
            alarm(timeout);
        }
        else{
            printf("Set frame was not prepared.\n");
            exit(1);
        }

    } else{
        printf("Failed to send C_Set %d times! Existing..\n",max_attempts);
        exit(1);
    }

}

int write_set_frame(int file_descriptor, int timeout_p, int num_atemps){
    (void) signal(SIGALRM, write_set);
    fd = file_descriptor;
    timeout = timeout_p;
    max_attempts = num_atemps;
    SET[0] = FLAG;
    SET[1] = A;
    SET[2] = C_SET;
    SET[3] = SET[1] ^ SET[2];
    SET[4] = FLAG;

    write_set();
    return 0;
}


