#ifndef LEME_OWNERSHIP_INTERNAL_H
#define LEME_OWNERSHIP_INTERNAL_H

struct leme_tag;
struct leme_view;

void leme_ownership_commit_tag(struct leme_view *view, struct leme_tag *tag);
void leme_ownership_replace_tag(struct leme_view *view, struct leme_tag *source,
                                struct leme_tag *destination);
void leme_ownership_release_tag(struct leme_view *view, struct leme_tag *tag);

#endif
