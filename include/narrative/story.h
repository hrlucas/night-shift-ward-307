#ifndef STORY_H
#define STORY_H

#include <stdbool.h>

/**
 * @file story.h
 * @brief Flags one-shot da história: esta cena roteirizada já aconteceu?
 *        Um bit por evento, mais STORY_FLAG_DENIED_ONCE (marca que o jogador já
 *        negou a revelação do surto uma vez, para a próxima tentativa mostrar
 *        uma linha extra de dica).
 */

typedef enum {
    STORY_FLAG_NONE             = 0,
    STORY_FLAG_EVENT_MONITOR    = (1 << 0),
    STORY_FLAG_EVENT_RECEPTION  = (1 << 1),
    STORY_FLAG_EVENT_PHONE_SELF = (1 << 2),
    STORY_FLAG_EVENT_KNOCK      = (1 << 3),
    STORY_FLAG_EVENT_MORGUE     = (1 << 4),
    STORY_FLAG_DENIED_ONCE      = (1 << 5)
} StoryFlag;

typedef struct {
    int flags;
} StoryProgression;

void story_init(StoryProgression* story);
void story_set_flag(StoryProgression* story, StoryFlag flag);
bool story_has_flag(const StoryProgression* story, StoryFlag flag);

#endif /* STORY_H */
