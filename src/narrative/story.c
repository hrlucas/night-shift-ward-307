#include "narrative/story.h"
#include <stddef.h>

/* Zera todas as flags de história (nenhum evento ocorreu ainda). */
void story_init(StoryProgression* story) {
    if (story == NULL) return;

    story->flags = STORY_FLAG_NONE;
}

/* Liga o bit de uma flag (marca que aquele evento já aconteceu). */
void story_set_flag(StoryProgression* story, StoryFlag flag) {
    if (story == NULL) return;

    story->flags |= flag;
}

/* Retorna true se o bit da flag está ligado. */
bool story_has_flag(const StoryProgression* story, StoryFlag flag) {
    if (story == NULL) return false;

    return (story->flags & flag) != 0;
}
