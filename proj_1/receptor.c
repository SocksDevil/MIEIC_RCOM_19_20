/*Non-Canonical Input Processing*/

#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "utils.h"
#include "ll.h"


int main(int argc, char **argv) {
  int port = parse_arguments(argc, argv);

  int fd = llopen(port, RECEIVER);
  if (fd == -1) {
    printf("Something went wrong!\n");
    return -1;
  }

  printf("Sucess!\n");

  if(llclose(fd) != 0){
    printf("Failed to close serial port!\n");
    exit(1);
  }
  return 0;
}
