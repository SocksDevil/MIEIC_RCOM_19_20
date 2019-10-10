#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int parse_arguments(int argc, char *argv[]){
  if ((argc < 2) ||
      ((strcmp("0", argv[1]) != 0) &&
       (strcmp("1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial 1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  return strtol(argv[1], NULL, 10);
}