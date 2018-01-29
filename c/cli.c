#include "cli.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static const char *program = "benchmark-io-c";
static const unsigned long long default_bs = 4096;
static const unsigned long long default_count = 100000;

/* clang-format off */
static const char *help_message =
"Simple file I/O benchmark.\n"
"\n"
"Usage:\n"
"  %s [options] (--rread | --rwrite | --sread | --swrite) <workdir>\n"
"\n"
"Options:\n"
"  -h --help             Show this screen.\n"
"  -b BYTES, --bs=BYTES  Block size of each I/O [default: %llu].\n"
"  -c N, --count=N       Total number of I/Os [default: %llu].\n"
"  --rread               Issue random reads.\n"
"  --rwrite              Issue random writes.\n"
"  --sread               Issue sequential reads.\n"
"  --swrite              Issue sequential writes.\n"
"";
/* clang-format on */

config_s cli_parse(int argc, char *argv[]) {
  config_s config = {
      .program = program,
      .workdir = NULL,
      .io_type = NONE,
      .bs = default_bs,
      .count = default_count,
  };

  const struct option longopts[] = {
      {"help", no_argument, NULL, 'h'},
      {"bs", required_argument, NULL, 'b'},
      {"count", required_argument, NULL, 'c'},
      {"rread", no_argument, (int *)&config.io_type, RAND_READ},
      {"rwrite", no_argument, (int *)&config.io_type, RAND_WRITE},
      {"sread", no_argument, (int *)&config.io_type, SEQ_READ},
      {"swrite", no_argument, (int *)&config.io_type, SEQ_WRITE},
      {NULL, 0, NULL, 0},
  };

  char *endptr;
  int opt;
  while ((opt = getopt_long(argc, argv, "b:c:h", longopts, NULL)) != -1) {
    switch (opt) {
    case 'h':
      printf(help_message, config.program, default_bs, default_count);
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

/* clang-format off */
static const char *hello_message =
"%s; Simple file I/O benchmark.\n"
"\n"
"Config:\n"
"  Working directory  %s\n"
"  I/O type           %s\n"
"  Block size         %llu\n"
"  Count              %llu\n"
"\n"
"";
/* clang-format on */

void cli_hello(const config_s *config) {
  const char *io_types[] = {
      "Random read",
      "Random write",
      "Sequential read",
      "Sequential write",
  };
  const char *io_type = io_types[config->io_type - RAND_READ];
  printf(hello_message, config->program, config->workdir, io_type, config->bs,
         config->count);
}
