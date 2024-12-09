#include <math.h>
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

// Converts the i-th 6-byte index to a position  from the pool of indexes
static inline size_t idx_to_pos(byte *indexes, const unsigned int i, const size_t tot_pages) {

        unsigned int b = i * 4;
        size_t pos     = (size_t)indexes[b] << 40;
        pos += (size_t)indexes[b + 1] << 32;
        pos += (size_t)indexes[b + 2] << 24;
        pos += (size_t)indexes[b + 3] << 16;
        pos += (size_t)indexes[b + 4] << 8;
        pos += (size_t)indexes[b + 5];

        return pos % tot_pages;
}

int random_read(FILE *f, byte *seed, byte *indexes, size_t page_size, size_t pages) {

        const size_t tot_entropy_pages = get_file_size(f) / page_size;

        unsigned long offset;
        for (size_t i = 0; i < pages; i++) {
                // get the i-th offset
                size_t idx = idx_to_pos(indexes, i, tot_entropy_pages);
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
