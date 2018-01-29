#include "cli.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

/* clang-format off */
static const char help_message[] =
"Simple file I/O benchmark.\n"
"\n"
"Usage:\n"
"  benchmark-io-c [options] (--rread | --rwrite | --sread | --swrite) <workdir>\n"
"\n"
"Options:\n"
"  -h --help             Show this screen.\n"
"  -b BYTES, --bs=BYTES  Block size of each I/O [default: 4096].\n"
"  -c N, --count=N       Total number of I/Os [default: 100000].\n"
"  --rread               Issue random reads.\n"
"  --rwrite              Issue random writes.\n"
"  --sread               Issue sequential reads.\n"
"  --swrite              Issue sequential writes.\n"
"";
/* clang-format on */

static unsigned long long default_bs = 4096;
static unsigned long long default_count = 100000;

static const struct option longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"bs", required_argument, NULL, 'b'},
    {"count", required_argument, NULL, 'c'},
    {"rread", no_argument, NULL, RAND_READ},
    {"rwrite", no_argument, NULL, RAND_WRITE},
    {"sread", no_argument, NULL, SEQ_READ},
    {"swrite", no_argument, NULL, SEQ_WRITE},
    {0, 0, 0, 0},
};

config_s cli_parse(int argc, char *argv[]) {
  config_s config = {
      .program = "benchmark-io-c",
      .workdir = NULL,
      .io_type = NONE,
      .bs = default_bs,
      .count = default_count,
  };

  char *endptr;
  int opt;
  while ((opt = getopt_long(argc, argv, "b:c:h", longopts, NULL)) != -1) {
    switch (opt) {
    case 'h':
      printf(help_message);
      exit(EXIT_SUCCESS);
    case 'b':
      config.bs = strtoull(optarg, &endptr, 0);
      if (*endptr != '\0') {
        fprintf(stderr, "%s: invalid argument -- %s\n", config.program, optarg);
        fprintf(stderr, "`-b`, `--bs` option require an integer argument\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 'c':
      config.count = strtoull(optarg, &endptr, 0);
      if (*endptr != '\0') {
        fprintf(stderr, "%s: invalid argument -- %s\n", config.program, optarg);
        fprintf(stderr, "`-c`, `--count` option require an integer argument\n");
        exit(EXIT_FAILURE);
      }
      break;
    case RAND_READ:
    case RAND_WRITE:
    case SEQ_READ:
    case SEQ_WRITE:
      config.io_type = opt;
      break;
    case '?':
      exit(EXIT_FAILURE);
    }
  }
  if (config.io_type == NONE) {
    fprintf(stderr,
            "%s: I/O type must be specified out of "
            "`--rread`, `--rwrite`, `--sread`, `--swrite`\n",
            config.program);
    exit(EXIT_FAILURE);
  }

  argc -= optind;
  argv += optind;

  if (argc < 1) {
    fprintf(stderr, "%s: argument `<workdir>` must be specified\n",
            config.program);
    exit(EXIT_FAILURE);
  }
  config.workdir = argv[0];

  return config;
}
