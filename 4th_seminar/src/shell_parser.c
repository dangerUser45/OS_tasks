#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

//--------------------------------------------------------------
static char* xstrndup(const char *s, size_t n) {
  char* p = (char*)calloc(n + 1, sizeof(char));
  if (!p) return NULL;
  memcpy(p, s, n);
  p[n] = '\0';
  return p;
}
//--------------------------------------------------------------
static void free_cmd(struct Cmd *c) {
  if (!c || !c->argv) return;
  for (int i = 0; i < c->argc; ++i) free(c->argv[i]);
  free(c->argv);
  c->argv = NULL;
  c->argc = 0;
}
//--------------------------------------------------------------
void free_pipeline(struct Pipeline *p) {
  if (!p) return;
  if (p->cmds) {
    for (int i = 0; i < p->ncmds; ++i) free_cmd(&p->cmds[i]);
    free(p->cmds);
  }
  free(p->in_path);
  free(p->out_path);
  memset(p, 0, sizeof(*p));
}
//--------------------------------------------------------------
static int push_new_cmd(struct Pipeline *pl, int *cap) {
  if (pl->ncmds == *cap) {
    int newcap = (*cap == 0) ? 4 : (*cap * 2);
    struct Cmd *tmp = (struct Cmd *)realloc(pl->cmds, sizeof(struct Cmd) * newcap);
    if (!tmp) return -1;
    for (int i = *cap; i < newcap; ++i) tmp[i] = (struct Cmd){0, NULL};
    pl->cmds = tmp;
    *cap = newcap;
  }
  pl->cmds[pl->ncmds] = (struct Cmd){0, NULL};
  pl->ncmds += 1;
  return 0;
}
//--------------------------------------------------------------
static int push_arg(struct Cmd *cmd, const char *beg, size_t len) {
  if (len == 0) return 0;
  char **newv = (char **)realloc(cmd->argv, sizeof(char *) * (cmd->argc + 2));
  if (!newv) return -1;
  cmd->argv = newv;
  cmd->argv[cmd->argc] = xstrndup(beg, len);
  if (!cmd->argv[cmd->argc]) return -1;
  cmd->argc += 1;
  cmd->argv[cmd->argc] = NULL;
  return 0;
}
//--------------------------------------------------------------
static const char *skip_ws(const char *p) {
  while (*p && isspace((unsigned char)*p)) ++p;
  return p;
}
//--------------------------------------------------------------
int parse_pipeline_simple(const char *line, struct Pipeline *out) {
  if (!line || !out) return -1;
  memset(out, 0, sizeof(*out));

  struct Pipeline pl = (struct Pipeline){0};
  int cap = 0;
  if (push_new_cmd(&pl, &cap) != 0) return -2;
  struct Cmd *cur = &pl.cmds[pl.ncmds - 1];

  const char *p = line;
  const char *tok_beg = NULL;
  int saw_gt = 0;
  int saw_lt = 0;
  int last_was_pipe = 0;

  while (true) {
    char c = *p;
    int is_end = (c == '\0');
    int is_sep = is_end || isspace((unsigned char)c)
                        || c == '|'
                        || c == '<'
                        || c == '>';
    if (!is_sep) {
      if (!tok_beg) tok_beg = p;
      ++p;
      continue;
    }

    if (tok_beg) {
      if (push_arg(cur, tok_beg, (size_t)(p - tok_beg)) != 0) {
        free_pipeline(&pl);
        return -3;
      }
      tok_beg = NULL;
      last_was_pipe = 0;
    }

    if (is_end) break;

    if (isspace((unsigned char)c)) {
      ++p;
      continue;
    }

    if (c == '|') {
      if (saw_gt) { free_pipeline(&pl); return -4; }
      if (cur->argc == 0 && pl.ncmds == 1) {
        free_pipeline(&pl); return -5;
      }
      if (push_new_cmd(&pl, &cap) != 0) {
        free_pipeline(&pl); return -2;
      }
      cur = &pl.cmds[pl.ncmds - 1];
      last_was_pipe = 1;
      ++p;
      continue;
    }

    if (c == '<' || c == '>') {
      int is_in = (c == '<');
      if (is_in) {
        if (pl.ncmds > 1) { free_pipeline(&pl); return -6; }
        if (saw_lt) { free_pipeline(&pl); return -7; }
        saw_lt = 1;
      } else {
        if (saw_gt) { free_pipeline(&pl); return -8; }
        saw_gt = 1;
      }

      ++p;
      p = skip_ws(p);
      if (*p == '\0' || *p == '|' || *p == '<' || *p == '>') {
        free_pipeline(&pl); return -9; }

      const char *name_beg = p;
      while (*p && !isspace((unsigned char)*p) && *p != '|'
                                               && *p != '<' 
                                               && *p != '>')
        ++p;
      size_t name_len = (size_t)(p - name_beg);
      char *path = xstrndup(name_beg, name_len);
      if (!path) {
        free_pipeline(&pl);
        return -3;
      }
      if (is_in) { 
        free(pl.in_path); pl.in_path = path;
      }
      else { 
        free(pl.out_path); pl.out_path = path;
      }
      continue;
    }

    ++p;
  }

  if (last_was_pipe && cur->argc == 0) {
    pl.trailing_stdin = 1;
    pl.ncmds -= 1;
  }

  *out = pl;
  return 0;
}
//--------------------------------------------------------------
