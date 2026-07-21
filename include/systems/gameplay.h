#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "data/game_state.h"

/**
 * @file gameplay.h
 * @brief Criação de personagem, o loop do turno em tempo real, os chamados de
 *        pacientes roteirizados e os três finais. Durante o turno é possível
 *        salvar em arquivo texto (tecla "0") e retomar depois pelo menu.
 */

typedef enum {
    GAMEPLAY_RESULT_ABANDONED,  /* jogador saiu no meio do turno (Esc) */
    GAMEPLAY_RESULT_ENDED       /* chegou a um dos três finais */
} GameplayResult;

/**
 * @brief Cria o personagem e joga um turno completo do começo.
 */
GameplayResult gameplay_start_new(GameData* data);

/**
 * @brief Retoma um turno a partir de um save carregado (sem criação de
 *        personagem nem intro). Retorna GAMEPLAY_RESULT_ABANDONED se o save
 *        não puder ser lido.
 */
GameplayResult gameplay_start_loaded(GameData* data);

#endif /* GAMEPLAY_H */
