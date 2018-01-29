#pragma once

typedef enum io_type_e {
  NONE,
  RAND_READ,
  RAND_WRITE,
  SEQ_READ,
  SEQ_WRITE,
} io_type_e;

typedef struct config_s {
  const char *program;
  const char *workdir;
  io_type_e io_type;
  unsigned long long bs;
  unsigned long long count;
} config_s;

config_s cli_parse(int argc, char *argv[]);
void cli_hello(const config_s *config);
