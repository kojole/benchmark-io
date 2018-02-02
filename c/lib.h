#pragma once

#include "cli.h"

#define _DEFAULT_SOURCE

#include <dirent.h>
#include <sys/types.h>
#include <time.h>

typedef struct io_log_s {
  struct timespec ts;
  int type;
  off_t at;
  size_t issue_bs;
  ssize_t complete_bs;
} io_log_s;

typedef struct bench_s {
  DIR *workdir_p;
  int target_fd;
  char *buffer;
  size_t logs_n;
  io_log_s *logs;
} bench_s;

bench_s setup(const config_s *config);
void run(const config_s *config, const bench_s *bench);
void teardown(const config_s *config, const bench_s *bench);