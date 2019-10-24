#include "disconnect_frame.h"

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "constants.h"
#include "utils.h"
static int fd = -1;
static int num_attempts = 4;
static int current_attempt = 0;
static int timeout = 0;
static unsigned char address_field = 0;

void write_disconnect() {
  if(current_attempt < num_attempts){
    send_non_info_frame(fd, C_DISC, address_field);
    alarm(timeout);
    current_attempt++;
  }else{
      printf("Failed to send disconnect, exiting\n");
      exit(1);
  }

}

void launch_disconnect_alarm(int file_descriptor, int new_timeout, unsigned char address) {
  timeout = new_timeout;
  fd = file_descriptor;
  address_field = address;
  signal(SIGALRM, write_disconnect);
  alarm(timeout);
  write_disconnect();
}