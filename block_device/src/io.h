#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdlib.h>

#include <blkio.h>

int read_random_pages(struct blkioq *q, unsigned int seed, int page_size,
                      uint64_t pages, void *buf);

int write(const char *path, void *buf, size_t size);                      

#endif /* IO_H */
