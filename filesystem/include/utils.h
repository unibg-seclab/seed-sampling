#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "types.h"

// returns the duration of a function F in nanoseconds
#define MEASURE(F)                                                                                 \
        ({                                                                                         \
                unsigned long long t = 0;                                                          \
                struct timespec start, end;                                                        \
                clock_gettime(CLOCK_MONOTONIC, &start);                                            \
                do {                                                                               \
                        F;                                                                         \
                } while (0);                                                                       \
                clock_gettime(CLOCK_MONOTONIC, &end);                                              \
                t = (long)(end.tv_sec - start.tv_sec) * 1000000000;                                \
                t += (end.tv_nsec - start.tv_nsec);                                                \
                t;                                                                                 \
        })

size_t get_file_size(FILE *fp);

// Returns a heap allocated `size`-bytes buffer of random memory read
// from /dev/urandom
byte *get_random_memory(size_t size);

// Writes a human readable representation of `size` into `buf`,
// assuming `size` is specified in bytes
void pprint_size(char *buf, size_t buf_size, size_t size);

// Converts a sequence of `secret_size`*2 characters `hex` into the
// sequence of `secret_size` bytes `secret`. Returns 0 in case of
// success, -1 in case of error.
int parse_hex(byte *secret, uint8_t secret_size, char *hex);

// Returns the minimum number of bits required to address all the
// `entropy_pages`. Returns 0 in case of error;
size_t get_index_size(size_t entropy_pages);

// Returns the minimum number of bytes required to store all the
// indexes.
size_t get_indexes_size(size_t index_size, size_t seed_pages);


#endif
