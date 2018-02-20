#pragma once

#include "cli.h"

#define _ATFILE_SOURCE 1
#define _BSD_SOURCE 1
#define _DEFAULT_SOURCE 1
#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <sys/types.h>
#include <time.h>

typedef struct io_log_s {
  struct timespec ts;
  off_t offset;
  ssize_t complete_bs;
} io_log_s;

typedef struct bench_s {
  DIR *workdir_p;
  int target_fd;
  char *buffer;
  struct timespec start;
  size_t logs_n;
  io_log_s *logs;
} bench_s;

bench_s setup(const config_s *config);
void run(const config_s *config, const bench_s *bench);
void teardown(const config_s *config, const bench_s *bench);
