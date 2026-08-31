#ifndef LEME_DIAGNOSTICS_H
#define LEME_DIAGNOSTICS_H

#include "config/scfg.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LEME_DIAGNOSTICS_MAX 64

enum leme_diagnostic_severity {
  LEME_DIAGNOSTIC_ERROR,
  LEME_DIAGNOSTIC_WARNING,
};

struct leme_diagnostic_span {
  uint16_t source;
  struct leme_scfg_span span;
  char *label;
};

struct leme_diagnostic {
  enum leme_diagnostic_severity severity;
  char *message;
  struct leme_diagnostic_span primary;
  struct leme_diagnostic_span *secondary;
  size_t secondary_count;
  char **notes;
  size_t notes_count;
  char **helps;
  size_t helps_count;
  uint16_t trail;
  uint16_t *iterations;
  size_t iterations_count;
  int line;
};

struct leme_diagnostic_detail {
  enum leme_diagnostic_severity severity;
  int line;
  struct leme_diagnostic_span primary;
  const char *label;
  const char *help;
  const char *note;
  uint16_t trail;
  struct leme_diagnostic_span secondary;
  const char *secondary_label;
  bool has_secondary;
};

struct leme_diagnostics {
  struct leme_diagnostic *entries;
  size_t count;
  size_t capacity;
  bool truncated;
};

bool leme_diagnostics_add(struct leme_diagnostics *diagnostics, int line,
                          const char *format, ...);
bool leme_diagnostics_add_detailed(struct leme_diagnostics *diagnostics,
                                   const struct leme_diagnostic_detail *detail,
                                   const char *format, ...);
void leme_diagnostics_finish(struct leme_diagnostics *diagnostics);

#endif
