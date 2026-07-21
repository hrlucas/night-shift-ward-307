#include "data/player.h"
#include <string.h>

/* Inicia o jogador com sanidade cheia e nome vazio. */
void player_init(Player* player) {
    if (player == NULL) return;

    memset(player->name, 0, sizeof(player->name));
    player->sanity = STARTING_SANITY;
    player->max_sanity = MAX_SANITY;
}

/* Soma `amount` à sanidade, limitando ao intervalo [0, max_sanity]. Aflição no
 * gameplay aplica valores negativos aqui. */
void player_modify_sanity(Player* player, int amount) {
    if (player == NULL) return;

    player->sanity = clamp_int(player->sanity + amount, MIN_SANITY, player->max_sanity);
}

/* Sanidade em porcentagem (0-100); a aflição exibida no HUD é 100 menos isto. */
int player_get_sanity_percent(const Player* player) {
    if (player == NULL || player->max_sanity == 0) return 0;

    return (player->sanity * 100) / player->max_sanity;
}
