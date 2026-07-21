#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

/**
 * @file state.h
 * @brief Definições da máquina de estados do jogo.
 */

/**
 * @brief Estados possíveis do jogo.
 */
typedef enum {
    STATE_BOOT,              /* Inicialização do jogo */
    STATE_TITLE_SCREEN,      /* Tela de título */
    STATE_MAIN_MENU,         /* Navegação no menu principal */
    STATE_PLAY,              /* Criação de personagem + sessão completa de jogo */
    STATE_SHUTDOWN           /* Encerramento do jogo */
} GameState;

/**
 * @brief Tipo de callback de transição de estado (on_enter/on_exit).
 */
typedef void (*StateCallback)(void);

/**
 * @brief Tipo de callback de atualização por quadro.
 */
typedef void (*StateUpdate)(float delta_time);

/**
 * @brief Tipo de callback de renderização.
 */
typedef void (*StateRender)(void);

/**
 * @brief Tipo de callback de tratamento de input.
 */
typedef void (*StateInput)(int input);

/**
 * @brief Descritor de um estado (callbacks agrupados).
 */
typedef struct {
    GameState state;
    StateCallback on_enter;
    StateCallback on_exit;
    StateUpdate update;
    StateRender render;
    StateInput handle_input;
} StateDescriptor;

/**
 * @brief Inicializa a máquina de estados.
 */
void state_machine_init(void);

/**
 * @brief Finaliza a máquina de estados.
 */
void state_machine_shutdown(void);

/**
 * @brief Registra o descritor de um estado.
 */
void state_register(GameState state, StateDescriptor descriptor);

/**
 * @brief Faz a transição para um novo estado.
 */
void state_transition(GameState new_state);

/**
 * @brief Retorna o estado atual.
 */
GameState state_get_current(void);

/**
 * @brief Atualiza o estado atual.
 */
void state_update(float delta_time);

/**
 * @brief Renderiza o estado atual.
 */
void state_render(void);

/**
 * @brief Trata input do estado atual.
 */
void state_handle_input(int input);

#endif /* STATE_H */
