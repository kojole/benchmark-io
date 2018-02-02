#include "cli.h"
#include "lib.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void *do_sleep(void *_arg) {
  sleep(5);
  return NULL;
}

static bench_s wait_for_setup(const config_s *config) {
  printf("Wait 5 seconds for setup.\n");

  pthread_t waiting_thread;
  int ret;
  ret = pthread_create(&waiting_thread, NULL, do_sleep, NULL);
  if (ret != 0) {
    fprintf(stderr, "%s: pthread_create(3) failed: %s\n", config->program,
            strerror(ret));
    exit(EXIT_FAILURE);
  }

  bench_s bench = setup(config);

  ret = pthread_join(waiting_thread, NULL);
  if (ret != 0) {
    fprintf(stderr, "%s: pthread_join(3) failed: %s\n", config->program,
            strerror(ret));
    exit(EXIT_FAILURE);
  }
  return bench;
}

int main(int argc, char *argv[]) {
  config_s config = cli_parse(argc, argv);
  cli_hello(&config);

  bench_s bench = wait_for_setup(&config);
  run(&config, &bench);
  teardown(&config, &bench);
}
