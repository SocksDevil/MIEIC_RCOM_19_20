/*Non-Canonical Input Processing*/

#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "utils.h"
#include "ll.h"
#include "app_layer.h"


int main(int argc, char **argv) {
  int port = parse_arguments(argc, argv);

  int fd = llopen(port, RECEIVER);
  if (fd == -1) {
    printf("Something went wrong!\n");
    return -1;
  }

  char filename[MAX_FILENAME_SIZE];
  if (read_file(fd, filename) == -1) {
    printf("Error reading file\n");
    return -1;
  }

  printf("Received file %s from serial port\n", filename);

  if(llclose(fd, RECEIVER) != 0){
    printf("Failed to close serial port!\n");
    exit(1);
  }
  return 0;
}
