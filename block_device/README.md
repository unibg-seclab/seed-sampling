# Keygen

## Requirements

- Install `libblkio`, the Block device I/O library, from source following the
  instructions in [its GitHub repository](https://gitlab.com/libblkio/libblkio)

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

  -r, --repetitions=INTEGER  Number of times that the extraction of the size is
                             done
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <seclab@unibg.it>.
```
