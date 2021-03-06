#include "lib.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef USE_FSYNC
#define fdatasync fsync
#endif

static void setup_rng(void);
static DIR *setup_workdir(const char *path);
static int setup_target(DIR *dir_p, size_t size);
static void setup_clear_cache(void);

static inline off_t random_offset(size_t bs, size_t count);

static void teardown_write_logs(const config_s *config, const bench_s *bench);
static void teardown_show_summary(const config_s *config, const bench_s *bench);
static void teardown_target(int fd);
static void teardown_workdir(DIR *dir_p);

static inline double ts2f(struct timespec ts) {
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

const char *program = NULL;

#define Error(format, ...)                                                     \
  do {                                                                         \
    printf("\n");                                                              \
    fprintf(stderr, "%s: " format, program, ##__VA_ARGS__);                    \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

bench_s setup(const config_s *config) {
  program = config->program;

  setup_rng();

  DIR *workdir_p = setup_workdir(config->workdir);

  printf("Preparing target file ... ");
  fflush(stdout);
  int target_fd = setup_target(workdir_p, config->filesize);
  printf("done.\n");

  if (config->clear_cache) {
    printf("Clearing page cache ... ");
    fflush(stdout);
    setup_clear_cache();
    printf("done.\n");
  } else {
    printf("Skip clearing page cache.\n");
  }

  bench_s bench = {
      .workdir_p = workdir_p,
      .target_fd = target_fd,
      .buffer = malloc(sizeof(char) * config->bs),
      .logs_n = config->count,
      .logs = malloc(sizeof(io_log_s) * config->count),
  };
  if (bench.buffer == NULL || bench.logs == NULL) {
    Error("setup: malloc(2) failed: %s\n", strerror(errno));
  }
  return bench;
}

static void setup_rng(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
    Error("setup: clock_gettime(3) failed: %s\n", strerror(errno));
  }
  srandom(ts.tv_sec ^ ts.tv_nsec);
}

static DIR *setup_workdir(const char *path) {
  DIR *workdir_p = opendir(path);
  if (workdir_p == NULL) {
    Error("setup: opendir(3) failed: %s\n", strerror(errno));
  }
  return workdir_p;
}

static int setup_target(DIR *dir_p, size_t size) {
  int dir_fd = dirfd(dir_p);
  if (dir_fd == -1) {
    Error("setup: dirfd(3) failed: %s\n", strerror(errno));
  }

  int target_fd = openat(dir_fd, "benchmark-io.bin", O_RDWR | O_CREAT, 0644);
  if (target_fd == -1) {
    Error("setup: openat(2) failed: %s\n", strerror(errno));
  }

  off_t actual_size = lseek(target_fd, 0, SEEK_END);
  if (actual_size == -1) {
    Error("setup: lseek(2) failed: %s\n", strerror(errno));
  }

  if (actual_size < size && ftruncate(target_fd, size) == -1) {
    Error("setup: ftruncate(2) failed: %s\n", strerror(errno));
  }

  if (lseek(target_fd, 0, SEEK_SET) == -1) {
    Error("setup: lseek(2) failed: %s\n", strerror(errno));
  }
  return target_fd;
}

static void setup_clear_cache(void) {
  int ret = system("sudo sysctl -w vm.drop_caches=3 > /dev/null");
  if (ret == -1) {
    Error("setup: system(2) failed\n");
  }
  if (WEXITSTATUS(ret) != 0) {
    Error("setup: failed to clean page cache: exit status = %d\n",
          WEXITSTATUS(ret));
  }
}

void run(const config_s *config, const bench_s *bench) {
  printf("\nRunning benchmark ... ");
  fflush(stdout);

  if (clock_gettime(CLOCK_MONOTONIC, (struct timespec *)&bench->start) == -1) {
    Error("run: clock_gettime(3) failed: %s\n", strerror(errno));
  }
  for (size_t i = 0; i < config->count; i++) {
    if (config->io_type & 0b10) {
      // random
      bench->logs[i].offset =
          random_offset(config->bs, config->filesize / config->bs);
      if (lseek(bench->target_fd, bench->logs[i].offset, SEEK_SET) == -1) {
        Error("run: lseek(2) faield: %s\n", strerror(errno));
      }
    } else {
      // sequential
      bench->logs[i].offset = config->bs * i;
    }
    if (config->io_type & 0b01) {
      // write
      if ((bench->logs[i].complete_bs =
               write(bench->target_fd, bench->buffer, config->bs)) == -1) {
        Error("run: write(2) failed: %s\n", strerror(errno));
      }
      if (fdatasync(bench->target_fd) == -1) {
        Error("run: fdatasync(2) or fsync(2) failed: %s\n", strerror(errno));
      }
    } else {
      // read
      if ((bench->logs[i].complete_bs =
               read(bench->target_fd, bench->buffer, config->bs)) == -1) {
        Error("run: read(2) failed: %s\n", strerror(errno));
      }
    }
#ifndef NO_EACH_ELAPSED_lOG
    if (clock_gettime(CLOCK_MONOTONIC, &bench->logs[i].ts) == -1) {
      Error("run: clock_gettime(3) failed: %s\n", strerror(errno));
    }
#endif
  }
  if (clock_gettime(CLOCK_MONOTONIC, (struct timespec *)&bench->finish) == -1) {
    Error("run: clock_gettime(3) failed: %s\n", strerror(errno));
  }

  printf("done.\n");
}

static inline off_t random_offset(size_t bs, size_t n_blocks) {
  return bs * (random() / (RAND_MAX / n_blocks + 1));
}

void teardown(const config_s *config, const bench_s *bench) {
  teardown_write_logs(config, bench);
  teardown_show_summary(config, bench);
  free(bench->logs);

  teardown_target(bench->target_fd);
  teardown_workdir(bench->workdir_p);
}

static void teardown_write_logs(const config_s *config, const bench_s *bench) {
  if (!config->write_log) {
    printf("Skip writing log.\n");
    return;
  }

  int dir_fd = dirfd(bench->workdir_p);
  if (dir_fd == -1) {
    Error("teardown: dirfd(3) failed: %s\n", strerror(errno));
  }

  time_t t = time(NULL);
  if (t == -1) {
    Error("teardown: time(2) failed: %s\n", strerror(errno));
  }
  struct tm *tm = localtime(&t);
  if (tm == NULL) {
    Error("teardown: localtime(3) failed: %s\n", strerror(errno));
  }

  char log_filename[64];
  if (strftime(log_filename, sizeof(log_filename),
               "benchmark-io_%F-%H-%M-%S.log", tm) == 0) {
    Error("teardown: strftime(3) failed\n");
  }

  printf("Writing log to %s ... ", log_filename);
  fflush(stdout);

  int log_fd = openat(dir_fd, log_filename, O_WRONLY | O_CREAT, 0644);
  if (log_fd == -1) {
    Error("teardown: openat(2) failed: %s\n", strerror(errno));
  }

  FILE *log_fp = fdopen(log_fd, "w");
  if (log_fp == NULL) {
    Error("teardown: fdopen(3) failed: %s\n", strerror(errno));
  }

  const char *io_types[] = {
      "SR",
      "SW",
      "RR",
      "RW",
  };
  fprintf(log_fp, "elapsed_time,io_type,offset,issue_bs,complete_bs\n");
  for (size_t i = 0; i < bench->logs_n; i++) {
#ifndef NO_EACH_ELAPSED_lOG
    const char *line = "%.9f,%s,%lld,%zu,%zu\n";
    double elapsed = ts2f(bench->logs[i].ts) - ts2f(bench->start);
#else
    const char *line = "%d,%s,%lld,%zu,%zu\n";
    int elapsed = -1;
#endif
    fprintf(log_fp, line, elapsed, io_types[config->io_type],
            (long long)bench->logs[i].offset, config->bs,
            bench->logs[i].complete_bs);
  }

  if (fclose(log_fp) == EOF) {
    Error("teardown: fclose(3) failed: %s\n", strerror(errno));
  }

  printf("done.\n");
}

/* clang-format off */
static const char *summary_message =
"\n"
"Summary:\n"
"  Elapsed time  %.3f [s]\n"
"  Throughput    %.3f [MiB/s]\n"
"  IOPS          %.3f\n"
"  Mean latency  %.3f [ms]\n"
"\n";
/* clang-format on */

static void teardown_show_summary(const config_s *config,
                                  const bench_s *bench) {
  int dir_fd = dirfd(bench->workdir_p);
  if (dir_fd == -1) {
    Error("teardown: dirfd(3) failed: %s\n", strerror(errno));
  }

  size_t total_complete_bs = 0;
  for (size_t i = 0; i < bench->logs_n; i++) {
    total_complete_bs += bench->logs[i].complete_bs;
  }

  const double elapsed_time =
#ifndef NO_EACH_ELAPSED_lOG
      ts2f(bench->logs[bench->logs_n - 1].ts) - ts2f(bench->start);
#else
      ts2f(bench->finish) - ts2f(bench->start);
#endif
  const double throughput_mib =
      total_complete_bs / (double)(1 << 20) / elapsed_time;
  const double iops = (double)config->count / elapsed_time;
  const double mean_latency_ms = elapsed_time / config->count * 1e3;
  printf(summary_message, elapsed_time, throughput_mib, iops, mean_latency_ms);
}

static void teardown_target(int fd) {
  if (close(fd) == -1) {
    Error("teardown: close(2) failed: %s\n", strerror(errno));
  }
}

static void teardown_workdir(DIR *dir_p) {
  if (closedir(dir_p) == -1) {
    Error("teardown: closedir(3) failed: %s\n", strerror(errno));
  }
}
