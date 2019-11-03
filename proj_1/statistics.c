#include "statistics.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "constants.h"

#define STATISTICS_FILENAME "statistics.csv"

static double begin;
static double end;
FILE *file;
void get_time(double * time){
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *time = ((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
}

void initialize_statistics() {
  if (access(STATISTICS_FILENAME, F_OK) != -1) {
    printf("Found file!\n");
    file = fopen(STATISTICS_FILENAME, "a+");
  }
  else {
    printf("Creating file!\n");
    file = fopen(STATISTICS_FILENAME, "w");
    fprintf(file, "Frame I Size, Baudrate, BCC/BBC2 Error Rate, Time\n");
  }

  fprintf(file, "%d, %d, %f/%f,", MAX_SIZE, BAUDRATE, BCC_ERROR_RATIO, BCC_ERROR_RATIO);
  get_time(&begin);
}

void store_final_time() {
  get_time(&end);
  fprintf(file, "%f\n", (end - begin)); // Time is stored in miliseconds
}