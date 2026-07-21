#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "core/utils.h"

/**
 * @file player.h
 * @brief Dados do jogador: identidade e sanidade (o medidor de aflição, ao
 *        contrário — aflição = 100 - sanidade%).
 */

typedef struct {
    char name[MAX_NAME_LENGTH];
    int sanity;
    int max_sanity;
} Player;

/**
 * @brief Inicializa o jogador com sanidade cheia e nome vazio.
 */
void player_init(Player* player);

/**
 * @brief Modifica a sanidade do jogador, limitada a [0, max_sanity].
 */
void player_modify_sanity(Player* player, int amount);

/**
 * @brief Retorna a sanidade do jogador como porcentagem (0-100).
 */
int player_get_sanity_percent(const Player* player);

#endif /* PLAYER_H */
