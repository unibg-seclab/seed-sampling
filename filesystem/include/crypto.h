#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/evp.h>
#include <stddef.h>
#include <stdint.h>
#include <types.h>

// Returns a heap-allocated array of `indexes_size` pseudo-random
// bytes. Uses the chacha20 cipher to generate the sequence,
// initializing it with with a 32 bytes key `key` and a 16 bytes iv
// `iv`. Returns NULL in case of error.
byte *generate_indexes(byte *key, byte *iv, size_t indexes_size);

#endif
