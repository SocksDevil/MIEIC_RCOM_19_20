#include "llinterpretation.h"
#include "constants.h"
#include <stdlib.h>
#include <stdio.h>

int interpreter(unsigned char *frame, char *buffer) {

  int size = 1;
  for (;; size++) {
    if (frame[size] == FLAG)
      break;
  }

  char bcc2 = frame[size - 1];

  int new_size = 0;
  for (int i = 4; i < size - 1; i++) {
    buffer[new_size] = frame[i];
    new_size++;
  }
  new_size++;
  
  char xor = buffer[0];
  for (int i = 1; i < new_size; i++) {
    xor ^= buffer[i];
  }

  printf("xor: %x, bcc2: %x\n", xor, bcc2);

  if (xor != bcc2)
    return -1;

  return new_size;
}