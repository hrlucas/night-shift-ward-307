#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "state.h"
#include "core/utils.h"

/**
 * @file game.h
 * @brief Loop principal do jogo e inicialização.
 */

/* Declaração adiantada */
typedef struct GameData GameData;

/**
 * @brief Configuração do jogo (capacidades do terminal).
 */
typedef struct {
    int terminal_width;
    int terminal_height;
    bool supports_color;
    bool supports_unicode;
} GameConfig;

/**
 * @brief Inicializa o jogo.
 */
bool game_init(void);

/**
 * @brief Finaliza o jogo.
 */
void game_shutdown(void);

/**
 * @brief Loop principal do jogo.
 */
void game_run(void);

/**
 * @brief Retorna a configuração do jogo.
 */
GameConfig* game_get_config(void);

/**
 * @brief Retorna os dados da partida.
 */
GameData* game_get_data(void);

/**
 * @brief Define os dados da partida.
 */
void game_set_data(GameData* data);

/**
 * @brief Reinicia o jogo para o estado inicial.
 */
void game_reset(void);

/**
 * @brief Detecta as capacidades do terminal.
 */
void game_detect_terminal(void);

#endif /* GAME_H */
