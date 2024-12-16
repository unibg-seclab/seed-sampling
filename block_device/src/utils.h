#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <time.h>

#define MAX(a,b)                                                               \
    ({ __typeof__ (a) _a = (a);                                                \
        __typeof__ (b) _b = (b);                                               \
        _a > _b ? _a : _b; })

#define MIN(a,b)                                                               \
    ({ __typeof__ (a) _a = (a);                                                \
        __typeof__ (b) _b = (b);                                               \
        _a > _b ? _a : _b; })

#define MEASURE(F)                                                             \
    ({                                                                         \
        double ms;                                                             \
        struct timespec start, end;                                            \
        clock_gettime(CLOCK_MONOTONIC, &start);                                \
        do {                                                                   \
                int ret = F;                                                   \
                if (ret) {                                                     \
                    exit_status = EXIT_FAILURE;                                \
                    goto unmap;                                                \
                }                                                              \
        } while (0);                                                           \
        clock_gettime(CLOCK_MONOTONIC, &end);                                  \
        ms = 1e3 * (end.tv_sec - start.tv_sec);                                \
        ms += 1e-6 * (end.tv_nsec - start.tv_nsec);                            \
        ms;                                                                    \
    })

#define OK(expr) OK_OR_GOTO(expr, clean)
#define OK_OR_GOTO(expr, label)                                                \
    do {                                                                       \
        int ret = (expr);                                                      \
        if (ret != 0) {                                                        \
            fprintf(stderr, "%s: %s: %s\n", #expr, strerror(-ret),             \
                    blkio_get_error_msg());                                    \
            exit_status = EXIT_FAILURE;                                        \
            goto label;                                                        \
        }                                                                      \
    } while (0)

int get_num_tokens(char *arg, const char *delimiter);
int parse_hex(char *hex, unsigned char *buf, size_t size);

#endif /* UTILS_H */
