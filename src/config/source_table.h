#ifndef LEME_CONFIG_SOURCE_TABLE_H
#define LEME_CONFIG_SOURCE_TABLE_H

#include "config/scfg.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LEME_SOURCE_TABLE_MAX 64U

struct leme_source_table {
  struct leme_scfg_source *entries;
  char **paths;
  size_t count;
};

bool leme_source_table_add(struct leme_source_table *table,
                           struct leme_scfg_source *source, const char *path,
                           uint16_t *index);
const struct leme_scfg_source *
leme_source_table_get(const struct leme_source_table *table, uint16_t index);
const char *leme_source_table_path(const struct leme_source_table *table,
                                   uint16_t index);
void leme_source_table_finish(struct leme_source_table *table);

#endif
