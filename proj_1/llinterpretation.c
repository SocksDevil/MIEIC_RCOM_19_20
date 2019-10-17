#include "llinterpretation.h"
#include "constants.h"
#include <stdlib.h>
#include <stdio.h>

int interpreter(unsigned char *frame, char *buffer) {

  int flag_i = 1;
  for (;; flag_i++) {
    if (frame[flag_i] == FLAG)
      break;
  }

  unsigned char bcc2 = frame[flag_i - 1];

  int new_size = 0;
  for (int i = 4; i < flag_i - 1; i++) {
    buffer[new_size] = frame[i];
    new_size++;
  }
  
  unsigned char xor = (unsigned char) buffer[0];
  for (int i = 1; i < new_size; i++) {
    xor ^= buffer[i];
  }

  printf("xor: %x, bcc2: %x\n", xor, bcc2);

  if (xor != bcc2)
    return -1;

  return new_size;
}