#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdlib.h>

#include <blkio.h>

int read_random_pages(struct blkioq *q, unsigned char *seed, int page_size,
                      uint64_t pages, uint64_t tot_pages, void *buf);

int write(const char *path, void *buf, size_t size);                      

#endif /* IO_H */
