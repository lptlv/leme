#ifndef LEME_CONFIG_RENDER_H
#define LEME_CONFIG_RENDER_H

#include "config/config.h"

char *leme_diagnostic_render_rich(const struct leme_config *config,
                                  const struct leme_diagnostic *diagnostic,
                                  bool colour);
char *leme_diagnostic_render_compact(const struct leme_config *config,
                                     const struct leme_diagnostic *diagnostic);

char *leme_diagnostic_render_rich_tables(
    const struct leme_source_table *sources,
    const struct leme_scfg_source *source_fallback,
    const struct leme_trail_table *trails, const char *default_path,
    const struct leme_diagnostic *diagnostic, bool colour);

#endif
