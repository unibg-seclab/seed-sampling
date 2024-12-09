#include <argp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <blkio.h>

#include "io.h"
#include "utils.h"

const char *argp_program_version = "1.0.0";
const char *argp_program_bug_address = "<seclab@unibg.it>";

enum args_key {
    ARG_KEY_SEED = 's',
    ARG_KEY_VERBOSE = 'v',
};

static struct argp_option options[] = {
    {"seed", ARG_KEY_SEED, "INTEGER", 0,
     "Seed to initialize the random number generation"},
    {"verbose", ARG_KEY_VERBOSE, NULL, 0, "Verbose mode"},
    {NULL},
};

error_t parse(int, char *, struct argp_state *);

const char args_doc[] = "DEVICE OUTPUT SIZE";

static struct argp argp = {options, parse, args_doc,
                           "keygen -- extract bytes from a block device"};

struct cli_args_t {
    const char* device;
    const char* output;
    int seed;
    size_t size;
    bool verbose;
};

error_t parse(int key, char *arg, struct argp_state *state) {
    struct cli_args_t *arguments = state->input;
    switch (key) {
    case ARG_KEY_SEED:
        arguments->seed = strtol(arg, NULL, 10);
        break;
    case ARG_KEY_VERBOSE:
        arguments->verbose = true;
        break;
    case ARGP_KEY_ARG:
        // 3 input argument: device, output, and size
        if (state->arg_num == 0)
            arguments->device = arg;
        else if (state->arg_num == 1)
            arguments->output = arg;
        else if (state->arg_num == 2)
            arguments->size = strtoull(arg, NULL, 10);
        else
            // Too many arguments, note that argp_usage exits
            argp_usage(state);
        break;
    case ARGP_KEY_END:
        if (state->arg_num < 3)
            // Too few arguments, note that argp_usage exits
            argp_usage(state);
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

int main(int argc, char **argv) {
    struct blkio *b;
    int exit_status = EXIT_SUCCESS;
    size_t size;
    size_t device_size;
    int opt_io_alignment;
    int opt_buf_alignment;
    int max_queues;
    int page_size;
    size_t buf_size;
    uint64_t pages;
    struct blkio_mem_region mem_region;
    void *buf;
    struct blkioq *q;

    struct cli_args_t args = {
        .device = NULL,
        .output = NULL,
        .seed = time(NULL),
        .size = 0,
        .verbose = false,
    };

    if (argp_parse(&argp, argc, argv, 0, 0, &args))
        return EXIT_FAILURE;
    
    OK(blkio_create("io_uring", &b));
    OK(blkio_set_str(b, "path", args.device));
    size = args.size;

    OK(blkio_connect(b));

    OK(blkio_get_uint64(b, "capacity", &device_size));
    OK(blkio_get_int(b, "optimal-io-alignment", &opt_io_alignment));
    OK(blkio_get_int(b, "optimal-buf-alignment", &opt_buf_alignment));
    OK(blkio_get_int(b, "max-queues", &max_queues));

    page_size = MAX(opt_io_alignment, opt_buf_alignment);
    buf_size = size + (size % page_size ? page_size - size % page_size : 0);
    pages = buf_size / page_size;

    if (args.verbose) {
        printf("Disk %s: %.2f GiB, %ld bytes, %ld sectors\n", args.device,
            (double) device_size / (1UL << 30), device_size,
            device_size / opt_io_alignment);
        printf("Optimal I/O alignment: %d bytes\n", opt_io_alignment);
        printf("Optimal I/O and memory buffer alignment: %d bytes\n", page_size);
    }

    OK(blkio_start(b));

    OK_OR_GOTO(blkio_alloc_mem_region(b, &mem_region, buf_size), unmap);
    buf = mem_region.addr;
    OK_OR_GOTO(blkio_map_mem_region(b, &mem_region), unmap);

    q = blkio_get_queue(b, 0);
    if (!q) {
        fprintf(stderr, "blkio_get_queue: Cannot get I/O queue\n");
        exit_status = EXIT_FAILURE;
        goto unmap;
    }

    if (args.verbose) printf("\n");

    printf("Read: %.2f GiB, %ld bytes, %ld pages\n",
           (double) buf_size / (1UL << 30), buf_size, pages);
    printf("Seed: %d\n", args.seed);
    printf("Finished in %.2f ms\n",
           MEASURE(read_random_pages(q, args.seed, page_size, pages, buf)));

    printf("\nWrite: %.2f GiB, %ld bytes\n", (double) size / (1UL << 30),
           size);
    printf("Finished in %.2f ms\n", MEASURE(write(args.output, buf, size)));

unmap:
    blkio_unmap_mem_region(b, &mem_region);
    blkio_free_mem_region(b, &mem_region);
clean:
    blkio_destroy(&b);
    return exit_status;
}
