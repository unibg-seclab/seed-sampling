#ifndef FSEEK_FUNCTIONS_H
#define FSEEK_FUNCTIONS_H

#include <stdio.h>

#include "types.h"
#include "utils.h"

// Performs a single fseek operation over the file `f`. Returns 0 in
// case of success, -1 in case of error.
int single_fseek(FILE *f);

// Performs a single fread operation over the file `f`, copying
// `page_size` bytes on the buffer `buf` Returns 0 in case of success,
// -1 in case of error.
int single_fread(FILE *f, byte *buf, size_t page_size);

// Copies into `seed` a portion of the file `f`. The scansion
// comprises the first `pages` pages, each long `page_size` bytes.
// Returns 0 in case of success, -1 in case of error.
int scan(FILE *f, byte *seed, size_t page_size, size_t pages);

// First transforms the buffer `indexes` into a sequence of
// `index_size`-bit long positions, which are used as page pointers
// into `f`. Then, for each position it copies a memory page of `f`
// into `seed`. Returns 0 in case of success, -1 in case of error.
int random_read(FILE *f, byte *seed, byte *indexes, size_t index_size, size_t page_size, size_t pages);

#endif
