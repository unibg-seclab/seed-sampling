#include <openssl/evp.h>
#include <stdlib.h>
#include <string.h>

#include "crypto.h"
#include "types.h"

byte *generate_indexes(byte *key, byte *iv, uint8_t index_size, size_t n) {

        if (index_size == 0)
                return NULL;

        byte *indexes = malloc(index_size * n);
        explicit_bzero(indexes, index_size * n);

        // init the cipher ctx
        EVP_CIPHER_CTX *cipher_ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit(cipher_ctx, EVP_chacha20(), key, iv);
        int outl = 0;
        // indexes generation
        EVP_EncryptUpdate(cipher_ctx, indexes, &outl, indexes, index_size * n);

        // ctx cleanup
        EVP_CIPHER_CTX_cleanup(cipher_ctx);
        EVP_CIPHER_CTX_free(cipher_ctx);

        return indexes;
}
