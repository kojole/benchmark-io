#include "cli.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  config_s config = cli_parse(argc, argv);
  cli_hello(&config);
}
