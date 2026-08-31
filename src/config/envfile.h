#ifndef LEME_CONFIG_ENVFILE_H
#define LEME_CONFIG_ENVFILE_H

#include "config/diagnostics.h"

#include <stdbool.h>
#include <stddef.h>

#define LEME_CONFIG_ENV_MAX_BYTES 65536U
#define LEME_CONFIG_ENV_MAX_LINE 4096U
#define LEME_CONFIG_ENV_MAX_ENTRIES 256U

struct leme_env_entry {
  char *name;
  char *value;
};

struct leme_env_file {
  struct leme_env_entry *entries;
  size_t count;
};

bool leme_env_file_load(struct leme_env_file *env, const char *config_path,
                        struct leme_diagnostics *diagnostics, char **error);
const char *leme_env_file_lookup(const struct leme_env_file *env,
                                 const char *name);
void leme_env_file_finish(struct leme_env_file *env);

#endif
