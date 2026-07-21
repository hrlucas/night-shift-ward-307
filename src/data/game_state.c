#include "data/game_state.h"

/* Inicializa uma sessão do zero: delega a inicialização de cada subsistema e
 * zera os contadores. Nada aqui é alocado no heap, então não há o que liberar. */
void game_data_init(GameData* data) {
    if (data == NULL) return;

    data->current_state = STATE_BOOT;

    player_init(&data->player);
    character_init(&data->character);
    time_init(&data->game_time);
    story_init(&data->story);

    pending_consequences_init(&data->pending_consequences);
    data->patient_scenarios_fired = 0;
    data->next_event_available_at = 0;
    data->last_ignore_minute = 0;

    data->current_room = 0;
    data->denial_index = 0;
    data->ignore_streak = 0;

    data->is_game_over = false;
}

/* Reinicia a sessão para começar uma partida nova (mesma coisa que init). */
void game_data_reset(GameData* data) {
    if (data == NULL) return;

    game_data_init(data);
}

/* Avança o relógio do jogo em `minutes` minutos. Ponto único por onde o tempo
 * passa, para ser fácil de rastrear. */
void game_data_advance_time(GameData* data, int minutes) {
    if (data == NULL) return;

    time_add_minutes(&data->game_time, minutes);
}
