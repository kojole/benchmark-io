#include "cli.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static size_t parse_sizearg(const char *optarg, const char *program,
                            const char *option) {
  char *endptr;
  size_t size = strtoull(optarg, &endptr, 0);
  if (*endptr != '\0') {
    fprintf(stderr, "%s: invalid argument -- %s\n", program, optarg);
    fprintf(stderr, "%s option requires an integer argument\n", option);
    exit(EXIT_FAILURE);
  }
  return size;
}

static const char *program = "benchmark-io-c";
static const size_t default_bs = 4096;
static const size_t default_count = 100000;
static const size_t default_filesize = 1 << 30;

/* clang-format off */
static const char *help_message =
"Simple file I/O benchmark.\n"
"\n"
"Usage:\n"
"  %s [options] (--rread | --rwrite | --sread | --swrite) <workdir>\n"
"\n"
"Options:\n"
"  -h --help             Show this screen.\n"
"  -b BYTES, --bs=BYTES  Block size of each I/O [default: %zu].\n"
"  -c N, --count=N       Total number of I/Os [default: %zu].\n"
"  --filesize-gib=GIB    Target file size in GiB [default: %zu].\n"
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
      .filesize = default_filesize,
  };

  const struct option longopts[] = {
      {"help", no_argument, NULL, 'h'},
      {"bs", required_argument, NULL, 'b'},
      {"count", required_argument, NULL, 'c'},
      {"filesize-gib", required_argument, NULL, 'f'},
      {"rread", no_argument, (int *)&config.io_type, RAND_READ},
      {"rwrite", no_argument, (int *)&config.io_type, RAND_WRITE},
      {"sread", no_argument, (int *)&config.io_type, SEQ_READ},
      {"swrite", no_argument, (int *)&config.io_type, SEQ_WRITE},
      {NULL, 0, NULL, 0},
  };

  int opt;
  int longindex = -1;
  while ((opt = getopt_long(argc, argv, "b:c:h", longopts, &longindex)) != -1) {
    switch (opt) {
    case 'h':
      printf(help_message, config.program, default_bs, default_count,
             default_filesize >> 30);
      exit(EXIT_SUCCESS);
    case 'b':
      config.bs = parse_sizearg(optarg, config.program,
                                (longindex == 1) ? "`--bs`" : "`-b`");
      break;
    case 'c':
      config.count = parse_sizearg(optarg, config.program,
                                   (longindex == 2) ? "`--count`" : "`-c`");
      break;
    case 'f':
      config.filesize =
          parse_sizearg(optarg, config.program, "`--filesize-gib`") << 30;
      break;
    case '?':
      exit(EXIT_FAILURE);
    }
  }
  if (config.bs * config.count > config.filesize) {
    fprintf(
        stderr,
        "%s: filesize must be greater than or equal to block size * count\n",
        config.program);
    fprintf(stderr, "  filesize           = %zu\n", config.filesize);
    fprintf(stderr, "  block size * count = %zu\n", config.bs * config.count);
    exit(EXIT_FAILURE);
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
"  Block size [byte]  %zu\n"
"  Count              %zu\n"
"  File size [GiB]    %zu\n"
"\n"
"";
/* clang-format on */

void cli_hello(const config_s *config) {
  const char *io_types[] = {
      "Sequential read",
      "Sequential write",
      "Random read",
      "Random write",
  };
  const char *io_type = io_types[config->io_type - SEQ_READ];
  printf(hello_message, config->program, config->workdir, io_type, config->bs,
         config->count, config->filesize >> 30);
}
