# Keygen

## Requirements

- `libblkio`: Install from source the Block device I/O library following these
  [instructions](https://gitlab.com/libblkio/libblkio)
- `libsodium`: Install this modern crypto library following these
  [instructions](https://doc.libsodium.org/installation)

## CLI

### Build

```shell
make keygen
```

### Usage

```shell
$ ./keygen --help
Usage: keygen [OPTION...] DEVICE OUTPUT SIZE
keygen -- extract bytes from a block device

  -b, --bypass-page-cache    Bypass the page cache of the device
  -s, --seed=INTEGER         Seed to initialize the random number generation
  -v, --verbose              Verbose mode
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <seclab@unibg.it>.
```

## Bench

### Build

```shell
make bench
```

### Usage

```shell
$ ./bench --help
Usage: bench [OPTION...] DEVICES SIZES OUTPUT
bench -- benchmark the extraction of bytes from block devices

  -b, --bypass-page-cache    Bypass the page cache of the device
  -r, --runs=INTEGER         Number of test runs for each device and size
                             configuration (default: 100)
  -w, --warm-runs=INTEGER    Number of test runs to warm-up the test
                             environment (default: 10)
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <seclab@unibg.it>.
```
