#include "config/diagnostics.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool leme_diagnostics_grow(struct leme_diagnostics *diagnostics) {
  struct leme_diagnostic *entries;
  size_t capacity;

  if (diagnostics->count < diagnostics->capacity) {
    return true;
  }
  capacity = diagnostics->capacity == 0 ? 8 : diagnostics->capacity * 2;
  if (capacity > LEME_DIAGNOSTICS_MAX) {
    capacity = LEME_DIAGNOSTICS_MAX;
  }
  entries = realloc(diagnostics->entries, capacity * sizeof(*entries));
  if (entries == NULL) {
    return false;
  }
  diagnostics->entries = entries;
  diagnostics->capacity = capacity;
  return true;
}

static char *leme_diagnostics_format(const char *format, va_list arguments) {
  va_list copy;
  char *message;
  int length;

  va_copy(copy, arguments);
  length = vsnprintf(NULL, 0, format, copy);
  va_end(copy);
  if (length < 0) {
    return NULL;
  }
  message = malloc((size_t)length + 1);
  if (message == NULL) {
    return NULL;
  }
  vsnprintf(message, (size_t)length + 1, format, arguments);
  return message;
}

static bool leme_diagnostics_push_string(char ***array, size_t *count,
                                         const char *text) {
  char **grown;
  char *copy;

  if (text == NULL) {
    return true;
  }
  grown = realloc(*array, (*count + 1) * sizeof(*grown));
  if (grown == NULL) {
    return false;
  }
  *array = grown;
  copy = strdup(text);
  if (copy == NULL) {
    return false;
  }
  (*array)[*count] = copy;
  (*count)++;
  return true;
}

static bool leme_diagnostics_contains_string(char **array, size_t count,
                                             const char *text) {
  size_t index;

  if (array == NULL || text == NULL) {
    return false;
  }
  for (index = 0; index < count; index++) {
    if (strcmp(array[index], text) == 0) {
      return true;
    }
  }
  return false;
}

static struct leme_diagnostic *
leme_diagnostics_find_match(struct leme_diagnostics *diagnostics,
                            const struct leme_diagnostic_detail *detail,
                            const char *message) {
  size_t index;

  for (index = 0; index < diagnostics->count; index++) {
    struct leme_diagnostic *entry = &diagnostics->entries[index];

    if (entry->severity == detail->severity &&
        entry->primary.source == detail->primary.source &&
        entry->primary.span.offset == detail->primary.span.offset &&
        entry->primary.span.length == detail->primary.span.length &&
        strcmp(entry->message, message) == 0) {
      return entry;
    }
  }
  return NULL;
}

static bool leme_diagnostics_merge(struct leme_diagnostic *entry,
                                   uint16_t trail) {
  uint16_t *grown;

  if (trail == 0) {
    return true;
  }
  if (entry->iterations_count == 0) {
    grown = malloc(2 * sizeof(*grown));
    if (grown == NULL) {
      return false;
    }
    grown[0] = entry->trail;
    grown[1] = trail;
    entry->iterations = grown;
    entry->iterations_count = 2;
    return true;
  }
  grown = realloc(entry->iterations,
                  (entry->iterations_count + 1) * sizeof(*grown));
  if (grown == NULL) {
    return false;
  }
  entry->iterations = grown;
  entry->iterations[entry->iterations_count] = trail;
  entry->iterations_count++;
  return true;
}

bool leme_diagnostics_add_detailed(struct leme_diagnostics *diagnostics,
                                   const struct leme_diagnostic_detail *detail,
                                   const char *format, ...) {
  va_list arguments;
  struct leme_diagnostic *match;
  struct leme_diagnostic *entry;
  char *message;

  va_start(arguments, format);
  message = leme_diagnostics_format(format, arguments);
  va_end(arguments);
  if (message == NULL) {
    return false;
  }

  match = leme_diagnostics_find_match(diagnostics, detail, message);
  if (match != NULL) {
    if (detail->help != NULL &&
        !leme_diagnostics_contains_string(match->helps, match->helps_count,
                                          detail->help)) {
      if (!leme_diagnostics_push_string(&match->helps, &match->helps_count,
                                        detail->help)) {
        free(message);
        return false;
      }
    }
    if (detail->note != NULL &&
        !leme_diagnostics_contains_string(match->notes, match->notes_count,
                                          detail->note)) {
      if (!leme_diagnostics_push_string(&match->notes, &match->notes_count,
                                        detail->note)) {
        free(message);
        return false;
      }
    }
    if (!leme_diagnostics_merge(match, detail->trail)) {
      free(message);
      return false;
    }
    free(message);
    return true;
  }

  if (diagnostics->count >= LEME_DIAGNOSTICS_MAX) {
    diagnostics->truncated = true;
    free(message);
    return true;
  }
  if (!leme_diagnostics_grow(diagnostics)) {
    free(message);
    return false;
  }
  entry = &diagnostics->entries[diagnostics->count];
  *entry = (struct leme_diagnostic){
      .severity = detail->severity,
      .message = message,
      .primary = detail->primary,
      .trail = detail->trail,
      .line = detail->line,
  };
  entry->primary.label = NULL;
  if (detail->label != NULL) {
    entry->primary.label = strdup(detail->label);
    if (entry->primary.label == NULL) {
      free(message);
      return false;
    }
  }
  if (!leme_diagnostics_push_string(&entry->helps, &entry->helps_count,
                                    detail->help)) {
    free(entry->primary.label);
    free(message);
    return false;
  }
  if (!leme_diagnostics_push_string(&entry->notes, &entry->notes_count,
                                    detail->note)) {
    free(entry->helps == NULL ? NULL : entry->helps[0]);
    free(entry->helps);
    free(entry->primary.label);
    free(message);
    return false;
  }
  if (detail->has_secondary) {
    entry->secondary = malloc(sizeof(*entry->secondary));
    if (entry->secondary == NULL) {
      goto fail;
    }
    entry->secondary[0] = detail->secondary;
    entry->secondary[0].label = NULL;
    if (detail->secondary_label != NULL) {
      entry->secondary[0].label = strdup(detail->secondary_label);
      if (entry->secondary[0].label == NULL) {
        goto fail;
      }
    }
    entry->secondary_count = 1;
  }
  diagnostics->count++;
  return true;

fail:
  free(entry->secondary);
  free(entry->notes == NULL ? NULL : entry->notes[0]);
  free(entry->notes);
  free(entry->helps == NULL ? NULL : entry->helps[0]);
  free(entry->helps);
  free(entry->primary.label);
  free(message);
  *entry = (struct leme_diagnostic){0};
  return false;
}

bool leme_diagnostics_add(struct leme_diagnostics *diagnostics, int line,
                          const char *format, ...) {
  const struct leme_diagnostic_detail detail = {
      .severity = LEME_DIAGNOSTIC_WARNING,
      .line = line,
  };
  va_list arguments;
  char *message;
  bool added;

  va_start(arguments, format);
  message = leme_diagnostics_format(format, arguments);
  va_end(arguments);
  if (message == NULL) {
    return false;
  }
  added = leme_diagnostics_add_detailed(diagnostics, &detail, "%s", message);
  free(message);
  return added;
}

void leme_diagnostics_finish(struct leme_diagnostics *diagnostics) {
  size_t index;
  size_t inner;

  for (index = 0; index < diagnostics->count; index++) {
    struct leme_diagnostic *entry = &diagnostics->entries[index];

    free(entry->message);
    free(entry->primary.label);
    for (inner = 0; inner < entry->secondary_count; inner++) {
      free(entry->secondary[inner].label);
    }
    free(entry->secondary);
    for (inner = 0; inner < entry->notes_count; inner++) {
      free(entry->notes[inner]);
    }
    free(entry->notes);
    for (inner = 0; inner < entry->helps_count; inner++) {
      free(entry->helps[inner]);
    }
    free(entry->helps);
    free(entry->iterations);
  }
  free(diagnostics->entries);
  *diagnostics = (struct leme_diagnostics){0};
}
