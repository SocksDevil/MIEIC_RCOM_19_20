#include "ll.h"
#include <stdio.h>
#include "serial_driver.h"

static link_layer layer;

int llopen(int port, connection_role role) {
  sprintf(layer.port, "/dev/ttyS%d", port);
  layer.baud_rate = BAUDRATE;
  layer.num_transmissions = 3;
  layer.timeout = 3;
  int fd = open_connection(layer);
  if(fd != -1){
    switch (role) {
        case TRANSMITTER:
            set_connection(layer);
            break;
        case RECEIVER:
            acknowledge_connection();
    }
  }
  return fd;
}

int llwrite(int fd, char * buffer, int length) {

  // stuff buffer
  // send buffer
  
  printf("%s%d", buffer, length);
      
  // TODO return number of written chars
  return fd;
}

int llclose(int fd) {
    return close_connection(fd);
}