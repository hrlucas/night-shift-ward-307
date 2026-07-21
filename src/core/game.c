#include "core/game.h"
#include "data/game_state.h"
#include "ui/input.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static GameConfig config;
static GameData* game_data = NULL;

/* Aloca e inicializa a struct única da partida. Usa calloc para zerar tudo,
 * inclusive bytes de padding. Retorna false se a alocação falhar. */
bool game_init(void) {
    game_detect_terminal();

    game_data = (GameData*)calloc(1, sizeof(GameData));
    if (game_data == NULL) {
        return false;
    }

    game_data_init(game_data);

    return true;
}

/* Libera a struct da partida ao encerrar. */
void game_shutdown(void) {
    if (game_data != NULL) {
        free(game_data);
        game_data = NULL;
    }
}

/* Loop principal da aplicação: atualiza/renderiza o estado atual e lê uma tecla
 * (bloqueante) para o estado tratar, até chegar em STATE_SHUTDOWN. O turno de
 * jogo em si roda inteiro dentro do on_enter de STATE_PLAY. */
void game_run(void) {
    if (game_data == NULL) return;

    while (state_get_current() != STATE_SHUTDOWN) {
        state_update(0.0f);
        state_render();

        InputKey key = input_get_key();
        state_handle_input((int)key);
    }

    state_machine_shutdown();
}

GameConfig* game_get_config(void) {
    return &config;
}

GameData* game_get_data(void) {
    return game_data;
}

void game_set_data(GameData* data) {
    game_data = data;
}

/* Reinicia os dados da partida (usado antes de começar/retomar um jogo). */
void game_reset(void) {
    if (game_data != NULL) {
        game_data_reset(game_data);
    }
}

/* Preenche a configuração com padrões seguros (80x24) e detecta capacidades do
 * terminal por plataforma. */
void game_detect_terminal(void) {
    config.terminal_width = 80;
    config.terminal_height = 24;
    config.supports_color = true;
    config.supports_unicode = true;
    
#ifdef _WIN32
    config.supports_unicode = true;
#else
    char* term = getenv("TERM");
    if (term != NULL && strstr(term, "256color") != NULL) {
        config.supports_color = true;
    }
#endif
}
