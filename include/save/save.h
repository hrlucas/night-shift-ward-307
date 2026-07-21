#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>
#include "data/game_state.h"

/**
 * @file save.h
 * @brief Salvamento em arquivo texto simples.
 *
 * Um único save fixo (save.txt), sem múltiplos slots. O formato é texto puro:
 * um valor por linha, escrito e lido exatamente na mesma ordem. A ideia é ser
 * fácil de explicar — "eu escrevo cada variável numa linha e leio de volta na
 * mesma sequência". Nada de formato binário ou biblioteca externa.
 */

/* Nome do arquivo de save, criado no diretório atual. */
#define SAVE_FILE_PATH "save.txt"

/* Grava o estado atual da partida em save.txt. Retorna true em caso de sucesso. */
bool save_game(const GameData* data);

/* Lê save.txt de volta para *data. Retorna true se leu tudo com sucesso. */
bool load_game(GameData* data);

/* True se existe um save.txt legível no disco. */
bool save_exists(void);

/* Apaga o save.txt (usado ao chegar a um dos finais). */
void save_delete(void);

#endif /* SAVE_H */
