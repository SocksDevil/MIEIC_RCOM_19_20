/*Non-Canonical Input Processing*/

#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "utils.h"
#include "ll.h"
#include "app_layer.h"


int main(int argc, char **argv) {
  connection_type conn_type = parse_arguments(argc, argv);

  int fd = llopen(conn_type.port_num, conn_type.role);
  if (fd == -1) {
    printf("Something went wrong!\n");
    return -1;
  }

  if(conn_type.role == TRANSMITTER){
    if (send_file(conn_type.filename, fd) == -1) {
      printf("Error sending file %s\n", conn_type.filename);
      return -1;
    }
    printf("Written file %s to serial port\n", conn_type.filename);
    
  } else {
    char filename[MAX_FILENAME_SIZE];
    if (read_file(fd, filename) == -1) {
      printf("Error reading file\n");
      return -1;
    }

    printf("Received file %s from serial port\n", filename);

  }


  if (llclose(fd, conn_type.role) != 0) {
    printf("Failed to close serial port!\n");
    exit(1);
  }

  return 0;
}
