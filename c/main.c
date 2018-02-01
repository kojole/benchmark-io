#include "cli.h"
#include "lib.h"

int main(int argc, char *argv[]) {
  config_s config = cli_parse(argc, argv);
  cli_hello(&config);

  // TODO: finish setup on fixed time
  bench_s bench = setup(&config);
  run(&config, &bench);
  teardown(&config, &bench);
}
