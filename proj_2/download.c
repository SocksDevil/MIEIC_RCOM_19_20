#include "hostname.h"

#include <stdlib.h>
#include <stdio.h>


int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: download address\n");
    exit(1);
  }

  printf("Server ip %s\n", get_ip(argv[1]));

}