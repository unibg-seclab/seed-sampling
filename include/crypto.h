#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/evp.h>
#include <stddef.h>
#include <stdint.h>
#include <types.h>

// Returns a heap-allocated array of `n` pseudo-random indexes each
// `index_size` bytes long. Uses the chacha20 cipher to generate the
// sequence, initializing it with with a 32 bytes key `key` and a 16
// bytes iv `iv`.
byte *generate_indexes(byte *key, byte *iv, uint8_t index_size, size_t n);

#endif
