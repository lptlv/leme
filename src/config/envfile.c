#include "config/envfile.h"

#include "config/internal.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool leme_env_sibling_path(const char *config_path, char **out_path,
                                  char **error) {
  const char *slash;
  size_t directory_length;
  size_t total;
  char *path;

  *out_path = NULL;
  if (config_path == NULL) {
    return true;
  }
  slash = strrchr(config_path, '/');
  directory_length = slash == NULL ? 0 : (size_t)(slash - config_path) + 1;
  total = directory_length + strlen("leme.env") + 1;
  path = malloc(total);
  if (path == NULL) {
    leme_config_set_error(error, "%s: out of memory", config_path);
    return false;
  }
  memcpy(path, config_path, directory_length);
  memcpy(path + directory_length, "leme.env", strlen("leme.env") + 1);
  *out_path = path;
  return true;
}

static bool leme_env_read_file(const char *path, char **data, size_t *length,
                               bool *absent, char **error) {
  FILE *file;
  char *buffer;
  size_t capacity = 4096;
  size_t used = 0;

  *absent = false;
  file = fopen(path, "rb");
  if (file == NULL) {
    if (errno == ENOENT) {
      *absent = true;
      return true;
    }
    leme_config_set_error(error, "%s: cannot read: %s", path, strerror(errno));
    return false;
  }
  buffer = malloc(capacity);
  if (buffer == NULL) {
    fclose(file);
    leme_config_set_error(error, "%s: out of memory", path);
    return false;
  }
  for (;;) {
    size_t got = fread(buffer + used, 1, capacity - used, file);

    used += got;
    if (used > LEME_CONFIG_ENV_MAX_BYTES) {
      free(buffer);
      fclose(file);
      leme_config_set_error(error, "%s: file exceeds %u bytes", path,
                            LEME_CONFIG_ENV_MAX_BYTES);
      return false;
    }
    if (got == 0) {
      break;
    }
    if (used == capacity) {
      char *grown = realloc(buffer, capacity * 2);

      if (grown == NULL) {
        free(buffer);
        fclose(file);
        leme_config_set_error(error, "%s: out of memory", path);
        return false;
      }
      buffer = grown;
      capacity *= 2;
    }
  }
  if (ferror(file)) {
    free(buffer);
    fclose(file);
    leme_config_set_error(error, "%s: cannot read", path);
    return false;
  }
  fclose(file);
  if (used == capacity) {
    char *grown = realloc(buffer, capacity + 1);

    if (grown == NULL) {
      free(buffer);
      leme_config_set_error(error, "%s: out of memory", path);
      return false;
    }
    buffer = grown;
  }
  buffer[used] = '\0';
  *data = buffer;
  *length = used;
  return true;
}

static bool leme_env_valid_name(const char *name, size_t length, size_t *bad) {
  size_t index;

  if (length == 0) {
    return false;
  }
  if (name[0] >= '0' && name[0] <= '9') {
    *bad = 0;
    return false;
  }
  for (index = 0; index < length; index++) {
    char c = name[index];

    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || c == '_') {
      continue;
    }
    *bad = index;
    return false;
  }
  return true;
}

static bool leme_env_append(struct leme_env_file *env, const char *name,
                            size_t name_length, const char *value,
                            size_t value_length, int line,
                            struct leme_diagnostics *diagnostics,
                            const char *path, char **error) {
  struct leme_env_entry *grown;
  size_t index;
  char *name_copy;
  char *value_copy;

  for (index = 0; index < env->count; index++) {
    if (strlen(env->entries[index].name) == name_length &&
        memcmp(env->entries[index].name, name, name_length) == 0) {
      char *replacement = malloc(value_length + 1);

      if (replacement == NULL) {
        leme_config_set_error(error, "%s: out of memory", path);
        return false;
      }
      memcpy(replacement, value, value_length);
      replacement[value_length] = '\0';
      free(env->entries[index].value);
      env->entries[index].value = replacement;
      leme_diagnostics_add(diagnostics, line,
                           "%s:%d: duplicate entry %.*s; last value wins", path,
                           line, (int)name_length, name);
      return true;
    }
  }
  if (env->count >= LEME_CONFIG_ENV_MAX_ENTRIES) {
    leme_config_set_error(error, "%s:%d: more than %u entries", path, line,
                          LEME_CONFIG_ENV_MAX_ENTRIES);
    return false;
  }
  grown = realloc(env->entries, (env->count + 1) * sizeof(*grown));
  if (grown == NULL) {
    leme_config_set_error(error, "%s: out of memory", path);
    return false;
  }
  env->entries = grown;
  name_copy = malloc(name_length + 1);
  value_copy = malloc(value_length + 1);
  if (name_copy == NULL || value_copy == NULL) {
    free(name_copy);
    free(value_copy);
    leme_config_set_error(error, "%s: out of memory", path);
    return false;
  }
  memcpy(name_copy, name, name_length);
  name_copy[name_length] = '\0';
  memcpy(value_copy, value, value_length);
  value_copy[value_length] = '\0';
  env->entries[env->count].name = name_copy;
  env->entries[env->count].value = value_copy;
  env->count++;
  return true;
}

static bool leme_env_parse_line(struct leme_env_file *env, const char *line,
                                size_t length, int lineno,
                                struct leme_diagnostics *diagnostics,
                                const char *path, char **error) {
  size_t start = 0;
  size_t end = length;
  size_t equals;
  size_t name_start;
  size_t name_end;
  size_t value_start;
  size_t value_end;
  size_t bad = 0;

  if (length > LEME_CONFIG_ENV_MAX_LINE) {
    leme_config_set_error(error, "%s:%d: line exceeds %u bytes", path, lineno,
                          LEME_CONFIG_ENV_MAX_LINE);
    return false;
  }
  if (memchr(line, '\0', length) != NULL) {
    size_t nul_offset =
        (size_t)((const char *)memchr(line, '\0', length) - line);
    leme_config_set_error(error, "%s:%d:%d: unexpected NUL byte", path, lineno,
                          (int)nul_offset + 1);
    return false;
  }
  while (start < end && (line[start] == ' ' || line[start] == '\t')) {
    start++;
  }
  while (end > start && (line[end - 1] == ' ' || line[end - 1] == '\t' ||
                         line[end - 1] == '\r')) {
    end--;
  }
  if (start == end || line[start] == '#') {
    return true;
  }
  if (end - start > 7 && memcmp(line + start, "export ", 7) == 0) {
    leme_config_set_error(
        error, "%s:%d:%d: unexpected 'export' prefix; write NAME=value", path,
        lineno, (int)start + 1);
    return false;
  }
  for (equals = start; equals < end && line[equals] != '='; equals++) {
  }
  if (equals == end) {
    leme_config_set_error(error, "%s:%d:%d: expected NAME=value", path, lineno,
                          (int)start + 1);
    return false;
  }
  name_start = start;
  name_end = equals;
  while (name_end > name_start &&
         (line[name_end - 1] == ' ' || line[name_end - 1] == '\t')) {
    name_end--;
  }
  if (name_end == name_start) {
    leme_config_set_error(error, "%s:%d:%d: expected an entry name before '='",
                          path, lineno, (int)equals + 1);
    return false;
  }
  if (!leme_env_valid_name(line + name_start, name_end - name_start, &bad)) {
    if (line[name_start] >= '0' && line[name_start] <= '9') {
      leme_config_set_error(error,
                            "%s:%d:%d: entry name must not begin with a digit",
                            path, lineno, (int)name_start + 1);
    } else {
      leme_config_set_error(error, "%s:%d:%d: invalid character in entry name",
                            path, lineno, (int)(name_start + bad) + 1);
    }
    return false;
  }
  value_start = equals + 1;
  value_end = end;
  while (value_start < value_end &&
         (line[value_start] == ' ' || line[value_start] == '\t')) {
    value_start++;
  }
  return leme_env_append(env, line + name_start, name_end - name_start,
                         line + value_start, value_end - value_start, lineno,
                         diagnostics, path, error);
}

bool leme_env_file_load(struct leme_env_file *env, const char *config_path,
                        struct leme_diagnostics *diagnostics, char **error) {
  char *path;
  char *data = NULL;
  size_t length = 0;
  size_t offset = 0;
  int lineno = 1;
  bool absent = false;
  bool valid = true;

  *env = (struct leme_env_file){0};
  if (!leme_env_sibling_path(config_path, &path, error)) {
    return false;
  }
  if (path == NULL) {
    return true;
  }
  if (!leme_env_read_file(path, &data, &length, &absent, error)) {
    free(path);
    return false;
  }
  if (absent) {
    free(path);
    return true;
  }
  if (length >= 3 && (unsigned char)data[0] == 0xEF &&
      (unsigned char)data[1] == 0xBB && (unsigned char)data[2] == 0xBF) {
    offset = 3;
  }
  while (offset <= length && valid) {
    size_t end = offset;

    while (end < length && data[end] != '\n') {
      end++;
    }
    if (end > offset || offset < length) {
      valid = leme_env_parse_line(env, data + offset, end - offset, lineno,
                                  diagnostics, path, error);
    }
    if (end >= length) {
      break;
    }
    offset = end + 1;
    lineno++;
  }
  free(data);
  free(path);
  if (!valid) {
    leme_env_file_finish(env);
  }
  return valid;
}

const char *leme_env_file_lookup(const struct leme_env_file *env,
                                 const char *name) {
  size_t index;

  if (env == NULL || name == NULL) {
    return NULL;
  }
  for (index = 0; index < env->count; index++) {
    if (strcmp(env->entries[index].name, name) == 0) {
      const char *value = env->entries[index].value;

      return value[0] == '\0' ? NULL : value;
    }
  }
  return NULL;
}

void leme_env_file_finish(struct leme_env_file *env) {
  size_t index;

  if (env == NULL) {
    return;
  }
  for (index = 0; index < env->count; index++) {
    free(env->entries[index].name);
    free(env->entries[index].value);
  }
  free(env->entries);
  *env = (struct leme_env_file){0};
}
