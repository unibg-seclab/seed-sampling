#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "utils.h"

#define PAGE_SIZE 4096L
#define PROBES 100

void *random_memory(size_t size) {
  void *mem = malloc(size);

  if (!mem)
    return NULL;

  int fd = open("/dev/urandom", O_RDONLY);
  size_t read_size = read(fd, mem, size);
  close(fd);

  if (read_size != size)
    return NULL;

  return mem;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: main FILE\n");
    return 1;
  }

  char *buffer = malloc(PAGE_SIZE);
  double tot_us = 0;

  FILE *f = fopen(argv[1], "r");
  size_t size = get_file_size(f);

  size_t *indeces = (size_t *)random_memory(PROBES * sizeof(size_t));

  for (int i = 0; i < PROBES; i++) {
    int page_i = indeces[i] % (size / PAGE_SIZE);

    unsigned long long ns = MEASURE({
      fseek(f, PAGE_SIZE * page_i, SEEK_SET);
      fread(buffer, PAGE_SIZE, 1, f);
    });

    tot_us += (double)ns / 1e3;
  }

  printf("Average single-page reading time over %d 4KiB pages: %.5f us\n", PROBES, tot_us / PROBES);

  free(indeces);
  fclose(f);
  free(buffer);
}
