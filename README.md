# benchmark-io

[![Build Status](https://travis-ci.org/kojole/benchmark-io.svg?branch=master)](https://travis-ci.org/kojole/benchmark-io)

> Simple file I/O benchmark in C and Rust

## Usage

```
$ ./c/benchmark-io-c -h
Simple file I/O benchmark.

Usage:
  benchmark-io-c [options] (--rread | --rwrite | --sread | --swrite) WORKDIR

Options:
  -h --help             Show this screen.
  -b BYTES, --bs=BYTES  Block size of each I/O [default: 4096].
  -c N, --count=N       Total number of I/Os [default: 100000].
  --filesize-gib=GIB    Target file size in GiB [default: 1].
  --rread               Issue random reads.
  --rwrite              Issue random writes.
  --sread               Issue sequential reads.
  --swrite              Issue sequential writes.
  --no-clear-cache      Skip clearing page cache in setup.
  --no-write-log        Skip writing I/O log.
```

### What does it do

1. Create a `GIB`-GiB file in `WORKDIR` named `benchmark-io.bin` if needed.
2. Clear page cache.
3. Issue `BYTES`-byte reads/writes to the file `N` times.
4. Write I/O log to `benchmark-io_yyyy-MM-dd-HH-mm-ss.log`.

### Compile options

#### `USE_FSYNC` (C)

Call `fsync(2)` instead of `fdatasync(2)` after each `write(2)` calls.

Usage: `$ make CFLAGS=DUSE_FSYNC`

#### `NO_EACH_ELAPSED_LOG` (C), `no_each_elapsed_log` (Rust)

Disable logging elapsed time of each I/Os.

Usage:

- C: `$ make CFLAGS=DNO_EACH_ELAPSED_LOG`
- Rust: `$ cargo build --features no_each_elapsed_log`

## Example

```
$ ./c/benchmark-io-c -c 10000 --rwrite /mnt/d/workdir
benchmark-io-c; Simple file I/O benchmark.

Config:
  Working directory  /mnt/d/workdir
  I/O type           Random write
  Block size [byte]  4096
  Count              10000
  File size [GiB]    100

Wait 5 seconds for setup.
Preparing target file ... done.
Clearing page cache ... done.

Running benchmark ... done.
Writing log to benchmark-io_2018-02-21-18-17-22.log ... done.

Summary:
  Elapsed time  40.812 [s]
  Throughput    0.957 [MiB/s]
  IOPS          245.026
  Mean latency  4.081 [ms]
```

## `helper.sh`

Run `benchmark-io` while mesuring system performance using sysstat.

### Usage

```
$ ./helper.sh -h
Run benchmark-io while mesuring system performance with sysstat.

Usage:
  ./helper.sh BIN ARG...
  ./helper.sh -h | --help

Arguments:
  BIN  path to benchmark-io binary
  ARG  arguments passed to benchmark-io
  

$ ./helper.sh ./c/benchmark-io-c -c 10000 --rwrite /mnt/d/workdir
./helper.sh: Run benchmark-io

# benchmark-io output is here...

./helper.sh: Write sar report in binary form to /mnt/d/workdir/benchmark-io_sar_2018-02-21-18-17-22.out
```
