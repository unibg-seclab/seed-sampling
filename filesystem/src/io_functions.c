#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "io_functions.h"
#include "types.h"
#include "utils.h"

int single_fseek(FILE *f) {

        long offset = 0;
        if (-1 == fseek(f, offset, SEEK_SET)) {
                printf("single_fseed: fseek failed\n");
                return -1;
        }

        return 0;
}

int single_fread(FILE *f, byte *buf, size_t page_size) {

        if (1 != fread(buf, page_size, 1, f)) {
                printf("single_fread: cannot read enough memory\n");
                return -1;
        }

        return 0;
}

int scan(FILE *f, byte *seed, size_t page_size, size_t pages) {
        // fseek
        long offset = 0;
        offset      = fseek(f, offset, SEEK_SET);
        if (offset == -1) {
                printf("scan: fseek failed\n");
                return -1;
        }
        // fread
        if (pages != fread(seed, page_size, pages, f)) {
                printf("scan: cannot read enough memory\n");
                return -1;
        }

        return 0;
}

// Converts the i-th index to a position  from the pool of indexes
static inline size_t idx_to_pos(byte *indexes, size_t index_size, const unsigned int i) {

        unsigned int b = i * 4;
        size_t pos     = 0;
        uint8_t power  = 0;
        uint8_t j      = 0;
        while (index_size > 8) {
                pos += (size_t)indexes[b + j] << power;
                index_size -= 8;
                power += 8;
                j += 1;
        }
        if (index_size > 0) {
                byte mask = 255 >> (8 - index_size);
                pos += (size_t)(indexes[b + j] & mask) << power;
        }

        return pos;
}

int random_read(FILE *f, byte *seed, byte *indexes, size_t index_size, size_t page_size,
                size_t pages) {

        const size_t tot_entropy_pages = get_file_size(f) / page_size;
        if (log2(tot_entropy_pages) < index_size) {
                printf("[err] not enough entropy pages\n");
                return -1;
        }

        unsigned long offset;
        for (size_t i = 0; i < pages; i++) {
                // get the i-th offset
                size_t idx = idx_to_pos(indexes, index_size, i);
                offset     = idx * page_size;
                // fseek
                if (-1 == fseek(f, offset, SEEK_SET)) {
                        printf("random_read: fseek failed\n");
                        return -1;
                }
                // fread
                if (1 != fread(seed + i * page_size, page_size, 1, f)) {
                        printf("random_read: cannot read enough memory\n");
                        return -1;
                }
        }

        return 0;
}
