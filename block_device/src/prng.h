#ifndef PRNG_H
#define PRNG_H

#include <stdint.h>
#include <stdlib.h>

struct random_ctx {
    unsigned char *buf;
    uint8_t nbytes;
    size_t pos;
    size_t size;
    uint64_t upper_bound;
};

void init_random_ctx(struct random_ctx *ctx, unsigned char* seed,
                     uint64_t random_numbers, uint64_t upper_bound);
int get_random_number(struct random_ctx *ctx, uint64_t *number);
void free_random_ctx(struct random_ctx *ctx);

#endif /* PRNG_H */
