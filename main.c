#include <stdio.h>
#include "utils.h"

#define PAGE_SIZE 4096
#define REPS 100

int main(int argc, char **argv) {
  srand(time(NULL));
  if (argc < 2) {
    printf("Usage: main FILE\n");
    return 1;
  }

  char *buffer = malloc(PAGE_SIZE);
  double tot_us = 0;

  FILE *f = fopen(argv[1], "r");
  size_t size = get_file_size(f);

  for (int i = 0; i < REPS; i++) {
    int offset = (PAGE_SIZE * rand()) % size;

    unsigned long long ns = MEASURE({
      fseek(f, offset, SEEK_SET);
      fread(buffer, PAGE_SIZE, 1, f);
    });

    tot_us += (double)ns / 1e3;
  }

  printf("Average single-page reading time over %d 4KiB pages: %.5f us\n", REPS, tot_us / REPS); 
  fclose(f);
  free(buffer);
}
