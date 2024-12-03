#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "utils.h"

#define PAGE_SIZE 4096L
#define PROBES 1024
#define REPS 10

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

size_t *random_memory2(size_t tot) {
  size_t *mem = malloc(tot * sizeof(size_t));

  if (!mem)
    return NULL;

  for (size_t i = 0; i < tot; i++) {
    mem[i] = (size_t)rand();
  }

  return mem;
}

void scan(FILE *f, byte *seed, size_t pages){
	long offset = 0;
	fseek(f, offset, SEEK_SET);
	fread(seed, PAGE_SIZE, pages, f);
}

void random_read(FILE *f, byte *buffer, size_t *indeces, size_t probes, size_t entropy_pages) {
  for (int i = 0; i < probes; i++) {
    size_t offset = PAGE_SIZE * indeces[i] % entropy_pages;

    fseek(f, offset, SEEK_SET);
    fread(buffer, PAGE_SIZE, 1, f);
  }
}

int main(int argc, char **argv) {
  srand(time(NULL));

  if (argc < 2) {
    printf("Usage: main FILE\n");
    return 1;
  }

  byte *buffer = malloc(PAGE_SIZE);
  double tot_us = 0;

  FILE *f = fopen(argv[1], "r");
  size_t size = get_file_size(f);
  size_t tot_pages = size / PAGE_SIZE;

  printf("Using /dev/urandom\n");

  for (int i = 0; i < REPS; i++) {
    size_t *indeces = (size_t *)random_memory(PROBES * sizeof(size_t));
    unsigned long long ns = MEASURE({
      random_read(f, buffer, indeces, PROBES, tot_pages);
    });
    free(indeces);

    tot_us += (double)ns / 1e3;
  }

  printf("Average page reading time (%d pages) over %d 4KiB pages: %.5f us\n", PROBES, REPS, tot_us / REPS);

  printf("Using rand()\n");

  for (int i = 0; i < REPS; i++) {
    size_t *indeces = random_memory2(PROBES * sizeof(size_t));
    unsigned long long ns = MEASURE({
      random_read(f, buffer, indeces, PROBES, tot_pages);
    });
    free(indeces);

    tot_us += (double)ns / 1e3;
  }

  printf("Average page reading time (%d pages) over %d 4KiB pages: %.5f us\n", PROBES, REPS, tot_us / REPS);

  fclose(f);
  free(buffer);
}
