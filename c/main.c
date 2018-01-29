#include "cli.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  config_s config = cli_parse(argc, argv);

  printf("program = %s\n", config.program);
  printf("workdir = %s\n", config.workdir);
  printf("io_type = %d\n", config.io_type);
  printf("bs      = %llu\n", config.bs);
  printf("count   = %llu\n", config.count);
}
