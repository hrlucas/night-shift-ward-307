#include "core/state.h"
#include <stdlib.h>
#include <stdio.h>

#define MAX_STATES 16

/* Tabela de estados indexada pelo valor do enum GameState, mais o estado atual
 * e o anterior. O estado atual só é lido via state_get_current(). */
static StateDescriptor state_descriptors[MAX_STATES];
static GameState current_state = (GameState)-1;
static GameState previous_state = (GameState)-1;

/* Zera todos os descritores (opcional; main.c registra os estados direto). */
void state_machine_init(void) {
    for (int i = 0; i < MAX_STATES; i++) {
        state_descriptors[i].state = (GameState)i;
        state_descriptors[i].on_enter = NULL;
        state_descriptors[i].on_exit = NULL;
        state_descriptors[i].update = NULL;
        state_descriptors[i].render = NULL;
        state_descriptors[i].handle_input = NULL;
    }
    current_state = STATE_BOOT;
}

/* Limpa os callbacks de todos os estados no encerramento. */
void state_machine_shutdown(void) {
    for (int i = 0; i < MAX_STATES; i++) {
        state_descriptors[i].on_enter = NULL;
        state_descriptors[i].on_exit = NULL;
        state_descriptors[i].update = NULL;
        state_descriptors[i].render = NULL;
        state_descriptors[i].handle_input = NULL;
    }
}

/* Grava o descritor de um estado no índice correspondente ao seu valor de enum. */
void state_register(GameState state, StateDescriptor descriptor) {
    if (state >= 0 && state < MAX_STATES) {
        state_descriptors[state] = descriptor;
        state_descriptors[state].state = state;
    }
}

/* Troca de estado: chama on_exit do atual, atualiza atual/anterior e chama
 * on_enter do novo. Transição para o mesmo estado é ignorada. */
void state_transition(GameState new_state) {
    if (new_state == current_state) {
        return;
    }

    if (new_state < 0 || new_state >= MAX_STATES) {
        return;
    }

    if (current_state >= 0 && current_state < MAX_STATES &&
        state_descriptors[current_state].on_exit != NULL) {
        state_descriptors[current_state].on_exit();
    }

    previous_state = current_state;
    current_state = new_state;

    if (state_descriptors[current_state].on_enter != NULL) {
        state_descriptors[current_state].on_enter();
    }
}

/* Retorna o estado atual (única forma de lê-lo de fora). */
GameState state_get_current(void) {
    return current_state;
}

/* Delegam ao callback correspondente do estado atual, se existir. */
void state_update(float delta_time) {
    if (state_descriptors[current_state].update != NULL) {
        state_descriptors[current_state].update(delta_time);
    }
}

void state_render(void) {
    if (state_descriptors[current_state].render != NULL) {
        state_descriptors[current_state].render();
    }
}

void state_handle_input(int input) {
    if (state_descriptors[current_state].handle_input != NULL) {
        state_descriptors[current_state].handle_input(input);
    }
}
