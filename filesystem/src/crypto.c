#include <openssl/evp.h>
#include <stdlib.h>
#include <string.h>

#include "crypto.h"
#include "types.h"

byte *generate_indexes(byte *key, byte *iv, size_t indexes_size) {

        if (indexes_size == 0)
                return NULL;

        byte *indexes = malloc(indexes_size);
        explicit_bzero(indexes, indexes_size);

        // init the cipher ctx
        EVP_CIPHER_CTX *cipher_ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit(cipher_ctx, EVP_chacha20(), key, iv);
        int outl = 0;
        // indexes generation
        EVP_EncryptUpdate(cipher_ctx, indexes, &outl, indexes, indexes_size);

        // ctx cleanup
        EVP_CIPHER_CTX_cleanup(cipher_ctx);
        EVP_CIPHER_CTX_free(cipher_ctx);

        return indexes;
}
