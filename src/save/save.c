#include "save/save.h"
#include "data/time.h"
#include "data/character.h"
#include "narrative/story.h"
#include "systems/patients.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Salvamento em arquivo texto. O formato é deliberadamente simples: cada
 * variável ocupa uma linha do arquivo, na ordem em que aparecem em save_game();
 * load_game() lê exatamente na mesma ordem. Assim dá para abrir o save.txt num
 * editor e ver, linha a linha, o estado da partida.
 */

/* ---- Helpers de leitura -------------------------------------------------- */

/* Lê uma linha inteira do arquivo para `buf` e remove o '\n' do fim. Usamos
 * fgets (não fscanf "%s") porque os nomes podem conter espaços, e "%s" pararia
 * no primeiro espaço. Retorna false no fim do arquivo. */
static bool read_line(FILE* f, char* buf, size_t size) {
    if (fgets(buf, (int)size, f) == NULL) {
        return false;
    }
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        buf[--len] = '\0';
    }
    return true;
}

/* Lê uma linha e a interpreta como inteiro. Retorna false se não houver linha. */
static bool read_int(FILE* f, int* out) {
    char line[64];
    if (!read_line(f, line, sizeof(line))) {
        return false;
    }
    *out = (int)strtol(line, NULL, 10);
    return true;
}

/* ---- Salvar -------------------------------------------------------------- */

bool save_game(const GameData* data) {
    /* Abre o arquivo para escrita (cria/zera). */
    FILE* f = fopen(SAVE_FILE_PATH, "w");
    if (f == NULL) {
        return false;
    }

    /* Jogador: nome (linha própria por causa dos espaços) e sanidade. */
    fprintf(f, "%s\n", data->player.name);
    fprintf(f, "%d\n", data->player.sanity);
    fprintf(f, "%d\n", data->player.max_sanity);

    /* Perfil do personagem. */
    fprintf(f, "%s\n", data->character.name);
    fprintf(f, "%d\n", data->character.birth_day);
    fprintf(f, "%d\n", data->character.birth_month);
    fprintf(f, "%d\n", data->character.birth_year);
    fprintf(f, "%d\n", data->character.age);
    fprintf(f, "%d\n", (int)data->character.sex);

    /* Relógio: basta salvar o total de minutos desde 00:00; hora/minuto são
     * derivados dele no carregamento. */
    fprintf(f, "%d\n", data->game_time.total_minutes);

    /* Progresso da história (bitmask de eventos já ocorridos). */
    fprintf(f, "%d\n", data->story.flags);

    /* Contadores do ritmo do turno e dos streaks de evasão. */
    fprintf(f, "%d\n", data->patient_scenarios_fired);
    fprintf(f, "%d\n", data->next_event_available_at);
    fprintf(f, "%d\n", data->last_ignore_minute);
    fprintf(f, "%d\n", data->current_room);
    fprintf(f, "%d\n", data->denial_index);
    fprintf(f, "%d\n", data->ignore_streak);
    fprintf(f, "%d\n", data->is_game_over ? 1 : 0);

    /* Fila de consequências pendentes: primeiro quantas estão ativas, depois os
     * campos de cada uma. Salvamos só as ativas para reconstruí-las com
     * pending_consequences_add() no load. */
    int pending_count = 0;
    for (int i = 0; i < MAX_PENDING_TREATMENTS; i++) {
        if (data->pending_consequences.items[i].active) {
            pending_count++;
        }
    }
    fprintf(f, "%d\n", pending_count);
    for (int i = 0; i < MAX_PENDING_TREATMENTS; i++) {
        const PendingConsequence* pc = &data->pending_consequences.items[i];
        if (!pc->active) {
            continue;
        }
        fprintf(f, "%d\n", pc->room_number);
        fprintf(f, "%d\n", (int)pc->consequence_text);
        fprintf(f, "%d\n", pc->delta);
        fprintf(f, "%d\n", pc->deadline_total_minutes);
    }

    fclose(f);
    return true;
}

/* ---- Carregar ------------------------------------------------------------ */

bool load_game(GameData* data) {
    /* Abre o arquivo para leitura. */
    FILE* f = fopen(SAVE_FILE_PATH, "r");
    if (f == NULL) {
        return false;
    }

    bool ok = true;

    /* Jogador — lê o nome com read_line (aceita espaços). */
    ok = ok && read_line(f, data->player.name, sizeof(data->player.name));
    ok = ok && read_int(f, &data->player.sanity);
    ok = ok && read_int(f, &data->player.max_sanity);

    /* Perfil do personagem. */
    ok = ok && read_line(f, data->character.name, sizeof(data->character.name));
    ok = ok && read_int(f, &data->character.birth_day);
    ok = ok && read_int(f, &data->character.birth_month);
    ok = ok && read_int(f, &data->character.birth_year);
    ok = ok && read_int(f, &data->character.age);
    int sex_value = 0;
    ok = ok && read_int(f, &sex_value);
    data->character.sex = (PlayerSex)sex_value;

    /* Relógio: reconstrói hora/minuto a partir do total de minutos salvo. */
    int total_minutes = 0;
    ok = ok && read_int(f, &total_minutes);
    time_init(&data->game_time);
    time_add_minutes(&data->game_time, total_minutes);

    /* Progresso e contadores. */
    ok = ok && read_int(f, &data->story.flags);
    ok = ok && read_int(f, &data->patient_scenarios_fired);
    ok = ok && read_int(f, &data->next_event_available_at);
    ok = ok && read_int(f, &data->last_ignore_minute);
    ok = ok && read_int(f, &data->current_room);
    ok = ok && read_int(f, &data->denial_index);
    ok = ok && read_int(f, &data->ignore_streak);
    int game_over = 0;
    ok = ok && read_int(f, &game_over);
    data->is_game_over = (game_over != 0);

    /* Fila de consequências pendentes: começa vazia e re-adiciona cada entrada
     * salva reaproveitando pending_consequences_add(). */
    pending_consequences_init(&data->pending_consequences);
    int pending_count = 0;
    ok = ok && read_int(f, &pending_count);
    if (pending_count < 0 || pending_count > MAX_PENDING_TREATMENTS) {
        pending_count = 0;
        ok = false;
    }
    for (int i = 0; i < pending_count && ok; i++) {
        int room = 0, text = 0, delta = 0, deadline = 0;
        ok = ok && read_int(f, &room);
        ok = ok && read_int(f, &text);
        ok = ok && read_int(f, &delta);
        ok = ok && read_int(f, &deadline);
        if (ok) {
            pending_consequences_add(&data->pending_consequences, room,
                                     (StringID)text, delta, deadline);
        }
    }

    fclose(f);
    return ok;
}

/* ---- Existência / remoção ------------------------------------------------ */

bool save_exists(void) {
    FILE* f = fopen(SAVE_FILE_PATH, "r");
    if (f == NULL) {
        return false;
    }
    fclose(f);
    return true;
}

void save_delete(void) {
    remove(SAVE_FILE_PATH);
}
