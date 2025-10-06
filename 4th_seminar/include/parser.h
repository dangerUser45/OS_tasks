#pragma once

#include <stddef.h>

struct Cmd {
  int argc;
  char **argv;
};

struct Pipeline {
  struct Cmd *cmds;
  int ncmds;
  char *in_path;
  char *out_path;
  int trailing_stdin;
};

int parse_pipeline_simple(const char *line, struct Pipeline *out);
void free_pipeline(struct Pipeline *p);
