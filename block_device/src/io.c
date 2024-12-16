#include "io.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <blkio.h>

#include "prng.h"

int read_random_pages(struct blkioq *q, unsigned char *seed, int page_size,
                      uint64_t pages, uint64_t tot_pages, void *buf) {
    struct random_ctx ctx;
    uint64_t page_id;
    struct blkio_completion *completions;
    int ret;

    init_random_ctx(&ctx, seed, pages, tot_pages);
    for (int i = 0; i < pages; i++) {
        ret = get_random_number(&ctx, &page_id);
        if (ret) {
            fprintf(stderr,
                    "get_random_number: The context run out of bytes\n");
            goto free;
        }

        blkioq_read(q, page_size * page_id, buf + page_size * i,
                    page_size, NULL, 0);
    }

    completions = malloc(sizeof(struct blkio_completion) * pages);
    
    ret = blkioq_do_io(q, completions, pages, pages, NULL);
    if (ret != pages) {
        fprintf(stderr, "blkioq_do_io: %s: %s\n", strerror(-ret),
                blkio_get_error_msg());
        ret = -ret;
        goto free;
    }

    for (int i = 0; i < pages; i++) {
        if (completions[i].ret) {
            fprintf(stderr, "%s: %s\n", strerror(-completions[i].ret),
                    completions[i].error_msg);
            ret = -completions[i].ret;
            goto free;
        }
    }

    ret = 0;

free:
    free(completions);
    free_random_ctx(&ctx);
    return ret;
}

int write(const char *path, void *buf, size_t size) {
    FILE *fp = NULL;
    int ret;

    fp = fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "fopen: %s\n", strerror(errno));
        return errno;
    }

    ret = fwrite(buf, size, 1, fp);
    if (ret != 1) {
        fprintf(stderr, "fwrite: %s\n", strerror(ferror(fp)));
        return ferror(fp);
    }

    ret = fclose(fp);
    if (ret) {
        fprintf(stderr, "fclose: %s\n", strerror(errno));
        return errno;
    }

    return 0;
}
