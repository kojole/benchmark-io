#include "lib.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static DIR *setup_workdir(const char *path);
static int setup_target(DIR *dir_p, size_t size);
static void setup_clear_cache(void);

static void teardown_workdir(DIR *dir_p);

const char *program = NULL;

#define Error_0(format)                                                        \
  do {                                                                         \
    printf("\n");                                                              \
    fprintf(stderr, "%s: " format, program);                                   \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define Error(format, ...)                                                     \
  do {                                                                         \
    printf("\n");                                                              \
    fprintf(stderr, "%s: " format, program, __VA_ARGS__);                      \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

bench_s setup(const config_s *config) {
  program = config->program;
  size_t target_size = config->bs * config->count;

  DIR *workdir_p = setup_workdir(config->workdir);

  printf("Preparing target file ... ");
  fflush(stdout);
  int target_fd = setup_target(workdir_p, target_size);
  printf("done.\n");

  printf("Clearing page cache ... ");
  fflush(stdout);
  setup_clear_cache();
  printf("done.\n");

  bench_s bench = {
      .workdir_p = workdir_p,
      .target_fd = target_fd,
      .logs_n = config->count + 1,
      .logs = malloc(sizeof(io_log_s) * (config->count + 1)),
  };
  if (bench.logs == NULL) {
    Error("setup: malloc(2) failed: %s\n", strerror(errno));
  }
  return bench;
}

static DIR *setup_workdir(const char *path) {
  DIR *workdir_p = opendir(path);
  if (workdir_p == NULL) {
    Error("setup: opendir(3) failed: %s\n", strerror(errno));
  }
  return workdir_p;
}

static int setup_target(DIR *dir_p, size_t expected_size) {
  int dir_fd = dirfd(dir_p);
  if (dir_fd == -1) {
    Error("setup: dirfd(3) failed: %s\n", strerror(errno));
  }

  int target_fd = openat(dir_fd, "benchmark-io.bin", O_WRONLY | O_CREAT);
  if (target_fd == -1) {
    Error("setup: openat(2) failed: %s\n", strerror(errno));
  }

  off_t actual_size = lseek(target_fd, 0, SEEK_END);
  if (actual_size == -1) {
    Error("setup: lseek(2) failed: %s\n", strerror(errno));
  }

  if (actual_size < expected_size &&
      ftruncate(target_fd, expected_size) == -1) {
    Error("setup: ftruncate(2) failed: %s\n", strerror(errno));
  }

  return target_fd;
}

static void setup_clear_cache(void) {
  int ret = system("sudo sysctl -w vm.drop_caches=3");
  if (ret == -1) {
    Error_0("setup: system(2) failed\n");
  }
  if (WEXITSTATUS(ret) != 0) {
    Error("setup: failed to clean page cache: exit status = %d\n",
          WEXITSTATUS(ret));
  }
}

void run(const config_s *config, const bench_s *bench) {}

void teardown(const config_s *config, const bench_s *bench) {
  // TODO: write logs
  free(bench->logs);

  teardown_workdir(bench->workdir_p);
}

static void teardown_workdir(DIR *dir_p) {
  if (closedir(dir_p) == -1) {
    Error("teardown: closedir(3) failed: %s\n", strerror(errno));
  }
}
