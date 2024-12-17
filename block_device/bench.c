#include <argp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <blkio.h>
#include <sodium.h>

#include "io.h"
#include "utils.h"

const char *argp_program_version = "1.0.0";
const char *argp_program_bug_address = "<seclab@unibg.it>";

enum args_key {
    ARG_KEY_BYPASS_PAGE_CACHE = 'b',
    ARG_KEY_RUNS = 'r',
    ARG_KEY_WARM_RUNS = 'w',
};

static struct argp_option options[] = {
    {"bypass-page-cache", ARG_KEY_BYPASS_PAGE_CACHE, NULL, 0,
     "Bypass the page cache of the device"},
    {"runs", ARG_KEY_RUNS, "INTEGER", 0,
     "Number of test runs for each device and size configuration (default: 100)"},
    {"warm-runs", ARG_KEY_WARM_RUNS, "INTEGER", 0,
     "Number of test runs to warm-up the test environment (default: 10)"},
    {NULL},
};

error_t parse(int, char *, struct argp_state *);

const char args_doc[] = "DEVICES SIZES OUTPUT";

static struct argp argp = {options, parse, args_doc,
                           "bench -- benchmark the extraction of bytes from block devices"};

struct cli_args_t {
    bool bypass_page_cache;
    char **devices;
    int num_devices;
    int num_sizes;
    const char *output;
    int test_runs;
    size_t *sizes;
    int warm_test_runs;
};

int parse_devices(char *arg, char **devices_ptr[]) {
    int n = get_num_tokens(arg, " ");
    char **devices;
    char *token;
    int i = 0;
    
    *devices_ptr = malloc(sizeof(char *) * n);
    devices = *devices_ptr;

    token = strtok(arg, " ");
    while (token) {
        devices[i++] = token;
        token = strtok(NULL, " ");
    };

    return n;
}

int parse_sizes(char *arg, size_t *sizes_ptr[]) {
    int n = get_num_tokens(arg, " ");
    size_t *sizes;
    char *token;
    int i = 0;
    
    *sizes_ptr = malloc(sizeof(size_t) * n);
    sizes = *sizes_ptr;

    token = strtok(arg, " ");
    while (token) {
        sizes[i++] = strtoull(token, NULL, 10);
        token = strtok(NULL, " ");
    };

    return n;
}

error_t parse(int key, char *arg, struct argp_state *state) {
    struct cli_args_t *arguments = state->input;
    char *token;

    switch (key) {
    case ARG_KEY_BYPASS_PAGE_CACHE:
        arguments->bypass_page_cache = true;
        break;
    case ARG_KEY_RUNS:
        arguments->test_runs = strtol(arg, NULL, 10);
        break;
    case ARG_KEY_WARM_RUNS:
        arguments->warm_test_runs = strtol(arg, NULL, 10);
        break;
    case ARGP_KEY_ARG:
        // 3 input argument: devices, sizes, output
        if (state->arg_num == 0)
            arguments->num_devices = parse_devices(arg, &arguments->devices);
        else if (state->arg_num == 1)
            arguments->num_sizes = parse_sizes(arg, &arguments->sizes);
        else if (state->arg_num == 2)
            arguments->output = arg;
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

void csv_header(FILE *fout) {
    fprintf(fout, "run,");               // Identifier of the repetition
    fprintf(fout, "device,");            // Device
    fprintf(fout, "bypass_page_cache,"); // Bypass page cache
    fprintf(fout, "device_size,");       // Size of the device in bytes
    fprintf(fout, "page_size,");         // Size of the page in bytes
    fprintf(fout, "size,");              // Extracted size in bytes
    fprintf(fout, "time\n");             // Execution time in ms
    fflush(fout);
}

void csv_line(FILE *fout, int repetition, const char *device,
              bool bypass_page_cache, size_t device_size, int page_size,
              size_t size, double time) {
    fprintf(fout, "%d,", repetition);
    fprintf(fout, "%s,", device);
    fprintf(fout, "%s,", bypass_page_cache ? "true" : "false");
    fprintf(fout, "%zu,", device_size);
    fprintf(fout, "%d,", page_size);
    fprintf(fout, "%zu,", size);
    fprintf(fout, "%f\n", time);
    fflush(fout);
}

int bench_device(FILE *fout, const char* device, bool bypass_page_cache,
                 size_t *sizes, int num_sizes, int warm_test_runs,
                 int test_runs) {
    size_t seed_size;
    unsigned char* seed;
    struct blkio *b;
    int exit_status = EXIT_SUCCESS;
    size_t device_size;
    int opt_io_alignment;
    int opt_buf_alignment;
    int max_queues;
    int page_size;
    uint64_t tot_pages;
    size_t size;
    size_t buf_size;
    uint64_t pages;
    struct blkio_mem_region mem_region;
    void *buf;
    struct blkioq *q;
    double ms;

    seed_size = randombytes_seedbytes();
    seed = malloc(seed_size);

    OK(blkio_create("io_uring", &b));
    OK(blkio_set_bool(b, "direct", bypass_page_cache));
    OK(blkio_set_str(b, "path", device));

    OK(blkio_connect(b));

    OK(blkio_get_uint64(b, "capacity", &device_size));
    OK(blkio_get_int(b, "optimal-io-alignment", &opt_io_alignment));
    OK(blkio_get_int(b, "optimal-buf-alignment", &opt_buf_alignment));
    OK(blkio_get_int(b, "max-queues", &max_queues));

    page_size = MAX(opt_io_alignment, opt_buf_alignment);
    tot_pages = device_size / page_size;

    printf("Disk %s: %.2f GiB, %zu bytes, %zu sectors\n", device,
        (double) device_size / (1UL << 30), device_size,
        device_size / opt_io_alignment);
    printf("Optimal I/O alignment: %d bytes\n", opt_io_alignment);
    printf("Optimal I/O and memory buffer alignment: %d bytes\n\n", page_size);

    OK(blkio_start(b));

    for (int i = 0; i < num_sizes; i++) {
        size = sizes[i];
        buf_size = size + (size % page_size ? page_size - size % page_size : 0);
        pages = buf_size / page_size;

        OK_OR_GOTO(blkio_alloc_mem_region(b, &mem_region, buf_size), unmap);
        buf = mem_region.addr;
        OK_OR_GOTO(blkio_map_mem_region(b, &mem_region), unmap);

        // TODO: Check if it make sense to get the queue every time we change
        // the I/O data buffers
        q = blkio_get_queue(b, 0);
        if (!q) {
            fprintf(stderr, "blkio_get_queue: Cannot get I/O queue\n");
            exit_status = EXIT_FAILURE;
            goto unmap;
        }

        printf("Benchmark: %.2f GiB, %zu bytes, %ld pages\n",
               (double) buf_size / (1UL << 30), buf_size, pages);

        // NOTE: Preliminary results show the initial runs are slower than the
        // following ones due to the device behavior. So, to measure the
        // expected performance of the system when operational we run a few warm
        // test runs.
        for (int run = 0; run < warm_test_runs; run++) {
            // Use an unpredictable sequence of bytes for the seed
            randombytes_buf(seed, seed_size);

            read_random_pages(q, seed, page_size, pages, tot_pages, buf);
        }

        for (int run = 0; run < test_runs; run++) {
            // Use an unpredictable sequence of bytes for the seed
            randombytes_buf(seed, seed_size);

            ms = MEASURE(read_random_pages(q, seed, page_size, pages, tot_pages,
                                           buf));
            csv_line(fout, run, device, bypass_page_cache, device_size,
                     page_size, size, ms);
        }

unmap:
        blkio_unmap_mem_region(b, &mem_region);
        blkio_free_mem_region(b, &mem_region);
    }

    printf("\n"); // separate tests on different devices with a new line

clean:
    blkio_destroy(&b);
    free(seed);
    return exit_status;
}

int main(int argc, char **argv) {
    int exit_status = EXIT_SUCCESS;
    FILE *fout;
    int ret;

    struct cli_args_t args = {
        .devices = NULL,
        .output = NULL,
        .test_runs = 100,
        .sizes = NULL,
        .warm_test_runs = 10,
    };

    if (argp_parse(&argp, argc, argv, 0, 0, &args)) {
        exit_status = EXIT_FAILURE;
        goto free;
    }

    fout = fopen(args.output, "w");
    if (!fout) {
        fprintf(stderr, "fopen: %s\n", strerror(errno));
        exit_status = EXIT_FAILURE;
        goto free;
    }

    csv_header(fout);

    for (int i = 0; i < args.num_devices; i++) {
        exit_status = bench_device(fout, args.devices[i],
                                   args.bypass_page_cache, args.sizes,
                                   args.num_sizes, args.warm_test_runs,
                                   args.test_runs);
        if (exit_status)
            break;
    }

    ret = fclose(fout);
    if (ret) {
        fprintf(stderr, "fclose: %s\n", strerror(errno));
        exit_status = EXIT_FAILURE;
        goto free;
    }

free:
    free(args.devices);
    free(args.sizes);

    return exit_status;
}
