#pragma once

#include <stddef.h>

typedef enum io_type_e {
  SEQ_READ = 0b00,
  SEQ_WRITE = 0b01,
  RAND_READ = 0b10,
  RAND_WRITE = 0b11,
  NONE,
} io_type_e;

typedef struct config_s {
  const char *program;
  const char *workdir;
  io_type_e io_type;
  size_t bs;
  size_t count;
  size_t filesize;
  int clear_cache;
  int write_log;
} config_s;

config_s cli_parse(int argc, char *argv[]);
void cli_hello(const config_s *config);
