#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include "core/utils.h"
#include "data/time.h"
#include "data/player.h"
#include "data/character.h"
#include "core/state.h"
#include "narrative/story.h"
#include "systems/patients.h"

/**
 * @file game_state.h
 * @brief A única struct que guarda uma sessão inteira de jogo. Nada aqui é
 *        alocado no heap, então init/reset nunca precisam liberar nada. É esta
 *        struct que o módulo de save serializa campo a campo.
 */

typedef struct GameData {
    GameState current_state;
    Player player;
    CharacterProfile character;
    GameTime game_time;
    StoryProgression story;
    PendingConsequenceList pending_consequences;
    int patient_scenarios_fired;
    int next_event_available_at;   /* primeiro minuto em que um novo evento pode disparar */
    int last_ignore_minute;        /* quando ignore_streak subiu por último (para o decaimento) */
    int current_room;
    int denial_index;
    int ignore_streak;
    bool is_game_over;
} GameData;

/**
 * @brief Inicializa uma sessão de jogo do zero.
 */
void game_data_init(GameData* data);

/**
 * @brief Reinicia os dados para começar uma sessão totalmente nova.
 */
void game_data_reset(GameData* data);

/**
 * @brief Avança o relógio do jogo e atualiza os sistemas dependentes.
 */
void game_data_advance_time(GameData* data, int minutes);

#endif /* GAME_STATE_H */
