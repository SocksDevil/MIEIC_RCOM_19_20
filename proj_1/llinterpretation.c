#include "llinterpretation.h"
#include "constants.h"
#include "stdlib.h"

int interpreter(unsigned char **frame, char *buffer) {

  int size = 1;
  for (;; size++) {
    if ((*frame)[size] == FLAG)
      break;
  }

  char bcc2 = (*frame)[size - 2];

  int new_size = 0;
  for (int i = 4; i < size - 2; i++) {
    buffer[new_size] = 0;
    new_size++;
  }
  new_size++;

  char xor = buffer[0];
  for (int i = 1; i < new_size; i++) {
    xor ^= buffer[i];
  }

  if (xor != bcc2)
    return -1;

  free(*frame);

  return new_size;
}