#include "prng.h"

#include <string.h>

#include <math.h>
#include <sodium.h>

void init_random_ctx(struct random_ctx *ctx, unsigned char* seed,
                    uint64_t random_numbers, uint64_t upper_bound) {
    ctx->nbytes = ceil(log2(upper_bound) / 8);

    // Extimate the size of random bytes to generate knowing that on average
    // only half of them produce a uniform distribution. To be conservative,
    // here we generete 20% more random bytes than needed.
    // TODO: To ensure we never run out of random bytes we need to be able to
    // extend the sequence of random bytes generated.
    // To do so, we should consider implementing the ChaCha20 generation with
    // OpenSSL, and then use keep track of the counter to extend the sequence
    ctx->size = 1.2 * random_numbers * ctx->nbytes;

    ctx->buf = malloc(ctx->size);
    ctx->pos = 0;
    ctx->upper_bound = upper_bound;

    randombytes_buf_deterministic(ctx->buf, ctx->size, seed);
}

int load64_le(struct random_ctx *ctx, uint64_t *num) {
    *num = 0;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    if (ctx->pos + ctx->nbytes > ctx->size)
        return 1;

    memcpy(num, ctx->buf + ctx->pos, ctx->nbytes);
    ctx->pos += ctx->nbytes;
#else
    for (uint8_t i = 0; i < ctx->nbytes; i++) {
        if (ctx->pos == ctx->size)
            return 1;

        *num |= ctx->buf[ctx->pos++] << (8 * i);
    }
#endif

    return 0;
}

int get_random_number(struct random_ctx *ctx, uint64_t *num) {
    int err;
    uint64_t min;
    uint64_t r;

    if (ctx->upper_bound < 2)
        return 0;

    min = (1UL + ~ctx->upper_bound) % ctx->upper_bound;  /* = 2**64 mod upper_bound */
    do {
        err = load64_le(ctx, &r);
        if (err)
            return err;
    } while (r < min);
    /* r is now clamped to a set whose size mod upper_bound == 0
     * the worst case (2**63+1) requires ~ 2 attempts */

    *num = r % ctx->upper_bound;

    return 0;
}

void free_random_ctx(struct random_ctx *ctx) {
    // printf("pos = %zu, size = %zu, perc = %f\n", ctx->pos, ctx->size,
    //        (double) ctx->pos / ctx->size);
    free(ctx->buf);
}
