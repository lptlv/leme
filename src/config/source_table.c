#include "config/source_table.h"

#include <stdlib.h>
#include <string.h>

bool leme_source_table_add(struct leme_source_table *table,
                           struct leme_scfg_source *source, const char *path,
                           uint16_t *index) {
  struct leme_scfg_source *entries;
  char **paths;
  char *copy;

  if (table->count >= LEME_SOURCE_TABLE_MAX) {
    return false;
  }
  entries = realloc(table->entries, (table->count + 1) * sizeof(*entries));
  if (entries == NULL) {
    return false;
  }
  table->entries = entries;
  paths = realloc(table->paths, (table->count + 1) * sizeof(*paths));
  if (paths == NULL) {
    return false;
  }
  table->paths = paths;
  copy = strdup(path == NULL ? "config" : path);
  if (copy == NULL) {
    return false;
  }
  table->entries[table->count] = *source;
  table->paths[table->count] = copy;
  *source = (struct leme_scfg_source){0};
  if (index != NULL) {
    *index = (uint16_t)table->count;
  }
  table->count++;
  return true;
}

const struct leme_scfg_source *
leme_source_table_get(const struct leme_source_table *table, uint16_t index) {
  if ((size_t)index >= table->count) {
    return NULL;
  }
  return &table->entries[index];
}

const char *leme_source_table_path(const struct leme_source_table *table,
                                   uint16_t index) {
  if ((size_t)index >= table->count) {
    return NULL;
  }
  return table->paths[index];
}

void leme_source_table_finish(struct leme_source_table *table) {
  size_t index;

  for (index = 0; index < table->count; index++) {
    leme_scfg_source_finish(&table->entries[index]);
    free(table->paths[index]);
  }
  free(table->entries);
  free(table->paths);
  *table = (struct leme_source_table){0};
}
