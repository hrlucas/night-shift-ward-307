#include "systems/gameplay.h"
#include "systems/patients.h"
#include "data/character.h"
#include "core/i18n.h"
#include "core/utils.h"
#include "ui/input.h"
#include "ui/renderer.h"
#include "ui/box.h"
#include "narrative/story.h"
#include "save/save.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Salas (mantidas propositalmente poucas: o jogador é sempre perguntado antes
 * de ser mandado a qualquer lugar; a história nunca o move em silêncio). */
enum {
    ROOM_MONITORING = 0,
    ROOM_RECEPTION,
    ROOM_MORGUE,
    ROOM_PATIENT
};

/* Limiares de tempo dos eventos roteirizados, em minutos de jogo desde 00:00.
 * São os cinco sustos atmosféricos iniciais; os 20 cenários de paciente
 * (patients.c) tocam o resto do turno no próprio cronograma. */
#define T_MONITOR_FLICKER    15
#define T_RECEPTION_CALL      35
#define T_PHONE_SELF          55
#define T_KNOCK               80
#define T_MORGUE_CALL        110

#define WIN_MINUTES          420  /* 07:00 */

/* Ritmo do relógio: 1 minuto de jogo = 1 segundo real, sempre. É simples de
 * explicar e, como o relógio PARA durante uma decisão (prompt_choice é uma
 * leitura bloqueante, sem tick de fundo), o turno nunca passa de 07:00
 * escondido atrás de um chamado ainda não respondido. */
#define REAL_MS_PER_GAME_MINUTE 1000
#define FAINT_STREAK_THRESHOLD  3

/* Frases ambientes de clima/aflição, disparadas aleatoriamente ao longo do
 * turno a partir deste minuto, para manter o jogador em alerta constante. */
#define T_AMBIENT_STINGER_START   20
#define STINGER_MIN_COOLDOWN_MIN   8
#define STINGER_BASE_CHANCE_DEN   40

/* Janela silenciosa obrigatória entre o fim de um evento interativo e o próximo
 * poder disparar, sorteada aleatoriamente nesta faixa (minutos de jogo). Sempre
 * vence o horário agendado de um evento, então os chamados nunca se empilham.
 * Ajustável. */
#define EVENT_GAP_MIN   15
#define EVENT_GAP_MAX   90

/* O ignore_streak cura devagar: cai um ponto depois destes minutos de jogo sem
 * nenhuma nova escolha evasiva, para um início azarado não ser sentença de
 * morte. */
#define IGNORE_DECAY_MINUTES   45

/* Penalidades das sub-escolhas de complicação (comuns a todo cenário). */
#define COMPLICATION_HELP_PENALTY   6
#define COMPLICATION_SNEAK_PENALTY 15

/* Cenas narrativas: texto que avança sozinho. */
#define NARRATIVE_BEAT_CHARS_PER_SEC  17
#define NARRATIVE_BEAT_MIN_MS       2500
#define NARRATIVE_BEAT_MAX_MS       9000

/* ---- pequenos helpers ---------------------------------------------------- */

static void narrative_line(const char* text) {
    printf("\n");
    ui_center_line(text);
    printf("\n");
}

static int get_affliction_percent(const GameData* data) {
    return 100 - player_get_sanity_percent(&data->player);
}

static void apply_affliction_delta(GameData* data, int delta) {
    player_modify_sanity(&data->player, -delta);
}

/* Marcada quando o jogador aperta Esc num momento em que a tela não é um menu —
 * por exemplo, durante uma cena narrativa, que consome a tecla para pular o
 * texto. O loop principal lê esta flag e abre o menu de pausa, para que o Esc
 * nunca seja "engolido" por uma cena em andamento. */
static bool pause_requested = false;

/* Avança o relógio do jogo enquanto uma cena de texto fica na tela, no ritmo
 * único de 1 minuto de jogo por segundo real. Compartilhado por todas as
 * esperas em tempo real abaixo. */
static void tick_interaction_time(GameData* data, long long* last_tick, long long* acc_ms) {
    sleep_ms(80);
    long long now = get_time_ms();
    *acc_ms += (now - *last_tick);
    *last_tick = now;

    while (*acc_ms >= REAL_MS_PER_GAME_MINUTE) {
        *acc_ms -= REAL_MS_PER_GAME_MINUTE;
        game_data_advance_time(data, 1);
    }
}

/* Espera `duration_ms` mostrando a cena, com o relógio andando 1:1, e pode ser
 * interrompida por qualquer tecla. */
static void narrative_wait(GameData* data, int duration_ms) {
    long long last_tick = get_time_ms();
    long long acc_ms = 0;
    long long elapsed_ms = 0;

    while (elapsed_ms < duration_ms) {
        if (input_has_input()) {
            /* Qualquer tecla pula a cena; se foi Esc, o loop principal abre o
             * menu de pausa em seguida em vez de descartar a tecla. */
            if (input_get_key() == INPUT_ESCAPE) {
                pause_requested = true;
            }
            break;
        }

        long long before = get_time_ms();
        tick_interaction_time(data, &last_tick, &acc_ms);
        elapsed_ms += (get_time_ms() - before);
    }
}

/* Mesma espera, mas NÃO avança o relógio. Usada só nas cenas de intro/abertura
 * antes do turno — do ponto de vista do jogador o plantão ainda não começou,
 * então esses segundos não podem contar contra ele. */
static void narrative_wait_pregame(int duration_ms) {
    long long elapsed_ms = 0;

    while (elapsed_ms < duration_ms) {
        if (input_has_input()) {
            input_get_key();
            break;
        }
        sleep_ms(80);
        elapsed_ms += 80;
    }
}

static int narrative_beat_duration_ms(const char* text) {
    int len = (int)strlen(text);
    int duration = (len * 1000) / NARRATIVE_BEAT_CHARS_PER_SEC;
    return clamp_int(duration, NARRATIVE_BEAT_MIN_MS, NARRATIVE_BEAT_MAX_MS);
}

/* Mostra uma linha puramente narrativa centralizada e a deixa na tela por um
 * tempo proporcional ao tamanho do texto (pulável por qualquer tecla), em vez
 * de forçar um "pressione qualquer tecla". O relógio continua andando 1:1
 * enquanto espera. Decisões de verdade nunca usam isto — elas passam por
 * prompt_choice, que bloqueia até uma tecla válida (e ali o relógio para). */
static void narrative_beat(GameData* data, const char* text) {
    narrative_line(text);
    fflush(stdout);
    narrative_wait(data, narrative_beat_duration_ms(text));
}

/* Igual a narrative_beat, mas só para a intro/abertura de pré-turno: o relógio
 * ainda não começou a contar do ponto de vista do jogador. */
static void narrative_beat_pregame(const char* text) {
    narrative_line(text);
    fflush(stdout);
    narrative_wait_pregame(narrative_beat_duration_ms(text));
}

/* ---- HUD ----------------------------------------------------------------- */

static const char* get_room_name(int room) {
    switch (room) {
        case ROOM_MONITORING: return i18n_get(STR_ROOM_MONITORING_NAME);
        case ROOM_RECEPTION:  return i18n_get(STR_ROOM_RECEPTION_NAME);
        case ROOM_MORGUE:     return i18n_get(STR_ROOM_MORGUE_NAME);
        case ROOM_PATIENT:    return i18n_get(STR_ROOM_PATIENT_NAME);
        default: return "";
    }
}

static void draw_hud(const GameData* data) {
    char time_str[16];
    char line[128];

    renderer_clear();
    time_format(&data->game_time, time_str, sizeof(time_str));

    printf("\n");
    ui_center_line(time_str);
    printf("\n\n");

    ui_center_line(get_room_name(data->current_room));
    printf("\n\n");

    snprintf(line, sizeof(line), "%s %d%%", i18n_get(STR_HUD_AFFLICTION), get_affliction_percent(data));
    ui_center_line(line);
    printf("\n\n");
    fflush(stdout);
}

/* Linha opcional "Quarto N" sob o HUD (omitida em casos de corredor, onde não
 * há número de quarto). */
static void draw_room_label(int room_number) {
    if (room_number > 0) {
        char line[64];
        printf("\n");
        snprintf(line, sizeof(line), "%s %d", i18n_get(STR_PATIENT_ROOM_LABEL), room_number);
        ui_center_line(line);
        printf("\n");
    }
}

/* ---- prompt de escolha que preserva o contexto --------------------------- */

/* Desenha o HUD, uma linha de contexto/pergunta opcional e as opções, e então
 * espera por uma escolha 1..n com uma LEITURA BLOQUEANTE. Enquanto essa decisão
 * está na tela, o relógio fica PARADO (não há tick de fundo) — só volta a andar
 * quando o jogador responde. Isso deixa a espera trivial de explicar e elimina
 * qualquer risco de o turno passar de 07:00 atrás de uma decisão.
 *
 * Em input inválido, REDESENHA o quadro inteiro com uma linha de erro no topo,
 * para o jogador nunca perder de vista a hora / sala / aflição ou as opções.
 * Retorna um índice base-1; um stdin fechado (EOF) faz input_get_key() devolver
 * INPUT_ESCAPE, que aqui vira default_index — assim o jogo nunca trava num
 * laço ocioso. */
static int prompt_choice(GameData* data, const char* context,
                         const char* const* options, int n, int default_index) {
    bool show_error = false;

    while (1) {
        draw_hud(data);

        if (show_error) {
            ui_center_line(i18n_get(STR_MSG_INVALID_OPTION_RETRY));
            printf("\n\n");
        }
        if (context != NULL) {
            ui_center_line(context);
            printf("\n\n");
        }
        for (int i = 0; i < n; i++) {
            ui_center_line(options[i]);
            printf("\n");
        }
        printf("\n");
        fflush(stdout);

        /* Leitura bloqueante: o relógio não anda aqui. */
        InputKey key = input_get_key();

        if (key == INPUT_ESCAPE) {
            return default_index;
        }

        int idx = 0;
        switch (key) {
            case INPUT_1: idx = 1; break;
            case INPUT_2: idx = 2; break;
            case INPUT_3: idx = 3; break;
            case INPUT_4: idx = 4; break;
            default: idx = 0; break;
        }
        if (idx >= 1 && idx <= n) {
            return idx;
        }
        show_error = true;
    }
}

/* ---- guia de jogo -------------------------------------------------------- */

/* Mostrado uma vez ao começar um plantão novo, antes da criação de personagem e
 * da história: explica o ritmo do relógio e as teclas (escolhas, salvar, pausa).
 * Não avança o relógio — o plantão ainda não começou. */
static void run_guide(void) {
    renderer_clear();
    printf("\n");
    ui_box_top(UI_BOX_WIDTH);
    ui_box_text(UI_BOX_WIDTH, i18n_get(STR_GUIDE_TITLE));
    ui_box_divider(UI_BOX_WIDTH);
    ui_box_empty(UI_BOX_WIDTH);
    ui_box_text_left(UI_BOX_WIDTH, i18n_get(STR_GUIDE_CLOCK));
    ui_box_text_left(UI_BOX_WIDTH, i18n_get(STR_GUIDE_KEYS));
    ui_box_text_left(UI_BOX_WIDTH, i18n_get(STR_GUIDE_PAUSED_ON_CHOICE));
    ui_box_text_left(UI_BOX_WIDTH, i18n_get(STR_GUIDE_SAVE));
    ui_box_text_left(UI_BOX_WIDTH, i18n_get(STR_GUIDE_ESC));
    ui_box_empty(UI_BOX_WIDTH);
    ui_box_text(UI_BOX_WIDTH, i18n_get(STR_CONTINUE_PROMPT));
    ui_box_empty(UI_BOX_WIDTH);
    ui_box_bottom(UI_BOX_WIDTH);
    renderer_refresh();
    input_wait_for_key();
}

/* ---- criação de personagem ----------------------------------------------- */

static void run_character_creation(GameData* data) {
    CharacterProfile* c = &data->character;
    char line[128];

    renderer_clear();
    printf("\n");
    ui_box_top(UI_BOX_WIDTH);
    ui_box_text(UI_BOX_WIDTH, i18n_get(STR_CC_TITLE));
    ui_box_bottom(UI_BOX_WIDTH);
    printf("\n");

    ui_center_line(i18n_get(STR_CC_NAME_PROMPT));
    fflush(stdout);
    input_get_line(line, sizeof(line));
    if (is_string_empty(line)) {
        safe_strcpy(c->name, "Anonimo", sizeof(c->name));
    } else {
        safe_strcpy(c->name, line, sizeof(c->name));
    }

    int day = 1, month = 1, year = 2000;
    int attempts = 0;
    while (attempts < 5) {
        ui_center_line(i18n_get(STR_CC_BIRTHDATE_PROMPT));
        fflush(stdout);
        input_get_line(line, sizeof(line));

        char digits[16];
        int di = 0;
        for (int i = 0; line[i] != '\0' && di < 15; i++) {
            if (line[i] >= '0' && line[i] <= '9') {
                digits[di++] = line[i];
            }
        }
        digits[di] = '\0';

        if (di == 8) {
            char dstr[3] = { digits[0], digits[1], '\0' };
            char mstr[3] = { digits[2], digits[3], '\0' };
            char ystr[5] = { digits[4], digits[5], digits[6], digits[7], '\0' };
            day = atoi(dstr);
            month = atoi(mstr);
            year = atoi(ystr);

            if (day >= 1 && day <= 31 && month >= 1 && month <= 12 &&
                year >= 1900 && year <= 2100) {
                break;
            }
        }

        ui_center_line(i18n_get(STR_CC_BIRTHDATE_INVALID));
        printf("\n");
        attempts++;
    }

    c->birth_day = day;
    c->birth_month = month;
    c->birth_year = year;
    c->age = character_calculate_age(day, month, year);

    printf("\n");
    {
        char age_line[64];
        snprintf(age_line, sizeof(age_line), "%s %d", i18n_get(STR_CC_AGE_LABEL), c->age);
        ui_center_line(age_line);
        printf("\n\n");
    }

    while (1) {
        ui_center_line(i18n_get(STR_CC_SEX_PROMPT));
        printf("\n");
        ui_center_line(i18n_get(STR_CC_SEX_MALE));
        printf("\n");
        ui_center_line(i18n_get(STR_CC_SEX_FEMALE));
        printf("\n\n");
        fflush(stdout);

        InputKey key = input_get_key();
        if (key == INPUT_1) { c->sex = PLAYER_SEX_MALE; break; }
        if (key == INPUT_2 || key == INPUT_ESCAPE) { c->sex = PLAYER_SEX_FEMALE; break; }

        renderer_clear();
        printf("\n");
        ui_center_line(i18n_get(STR_MSG_INVALID_OPTION_RETRY));
        printf("\n");
    }

    safe_strcpy(data->player.name, c->name, sizeof(data->player.name));

    renderer_clear();
    printf("\n");
    ui_center_line(i18n_get(STR_CC_CONFIRM));
    fflush(stdout);
    input_wait_for_key();
}

/* Cerimônia de pré-turno: o relógio está em 00:00 e nada checa condições de
 * vitória/derrota ainda, então estas cenas usam a variante _pregame, que não
 * mexe no relógio. */
static void run_intro(GameData* data) {
    (void)data;
    renderer_clear();
    printf("\n\n");
    narrative_beat_pregame(i18n_get(STR_INTRO_FIRST_SHIFT));

    renderer_clear();
    printf("\n\n");
    ui_center_line(i18n_get(STR_INTRO_SHIFT_LABEL));
    printf("\n");
    ui_center_line(i18n_get(STR_INTRO_SHIFT_HOURS));
    narrative_beat_pregame(i18n_get(STR_INTRO_GOOD_LUCK));
}

/* Abertura curta que estabelece o peso emocional das escolhas que virão, sem
 * revelar nada da reviravolta. Toca uma vez, só num turno novo (não no reinício
 * por negar o desmaio, que repete run_intro mas não esta). */
static void run_opening(GameData* data) {
    (void)data;
    renderer_clear();
    printf("\n\n");
    narrative_beat_pregame(i18n_get(STR_OPENING_1));

    renderer_clear();
    printf("\n\n");
    narrative_beat_pregame(i18n_get(STR_OPENING_2));

    renderer_clear();
    printf("\n\n");
    narrative_beat_pregame(i18n_get(STR_OPENING_3));
}

/* ---- resolução genérica engajar/evitar (o peso "certo/errado") ----------- */

/* Registra uma escolha evasiva: aumenta o streak e lembra quando, para que o
 * decaimento em gameplay_loop possa curá-lo após um trecho de bom comportamento. */
static void register_ignore(GameData* data) {
    data->ignore_streak++;
    data->denial_index++;
    data->last_ignore_minute = data->game_time.total_minutes;
}

/* engaged=true é a escolha corajosa (atender, abrir); engaged=false é a evasiva
 * (ignorar, não abrir). Evitar custa bem mais caro. */
static void resolve_choice(GameData* data, bool engaged) {
    if (engaged) {
        apply_affliction_delta(data, -3);
        data->ignore_streak = 0;
    } else {
        apply_affliction_delta(data, 12);
        register_ignore(data);
    }
}

/* ---- eventos atmosféricos iniciais (one-shot) ---------------------------- */

/* Algumas formas intercambiáveis de o jogador perceber que algo precisa de
 * atenção, para que os 20+ chamados do turno não se anunciem todos do mesmo
 * jeito. Sorteadas antes do conteúdo específico do chamado. */
static const StringID call_announcements[] = {
    STR_ANNOUNCE_PHONE_RECEPTION,
    STR_ANNOUNCE_INTERCOM,
    STR_ANNOUNCE_VOICE_HALLWAY,
    STR_ANNOUNCE_CALL_LIGHT,
    STR_ANNOUNCE_FOOTSTEPS,
};
#define CALL_ANNOUNCEMENT_COUNT (int)(sizeof(call_announcements) / sizeof(call_announcements[0]))

static void announce_incoming_call(GameData* data) {
    int pick = random_range(0, CALL_ANNOUNCEMENT_COUNT - 1);
    narrative_beat(data, i18n_get(call_announcements[pick]));
}

static void event_monitor_flicker(GameData* data) {
    story_set_flag(&data->story, STORY_FLAG_EVENT_MONITOR);
    draw_hud(data);
    narrative_beat(data, i18n_get(STR_EV_MONITOR_FLICKER));
    apply_affliction_delta(data, -1);
}

static void event_reception_call(GameData* data) {
    story_set_flag(&data->story, STORY_FLAG_EVENT_RECEPTION);

    draw_hud(data);
    announce_incoming_call(data);
    draw_hud(data);
    narrative_beat(data, i18n_get(STR_EV_CALL_RECEPTION));

    const char* opts[2] = { i18n_get(STR_OPT_GO_NOW), i18n_get(STR_OPT_IGNORE_CALL) };
    int go = prompt_choice(data, i18n_get(STR_PATIENT_GO_PROMPT), opts, 2, 2);

    if (go == 1) {
        data->current_room = ROOM_RECEPTION;
        draw_hud(data);
        narrative_beat(data, i18n_get(STR_EV_RECEPTION_EMPTY));
        apply_affliction_delta(data, 4);
        data->current_room = ROOM_MONITORING;
    } else {
        apply_affliction_delta(data, 3);
    }
}

static void event_phone_self(GameData* data) {
    story_set_flag(&data->story, STORY_FLAG_EVENT_PHONE_SELF);

    const char* opts[2] = { i18n_get(STR_OPT_ANSWER), i18n_get(STR_OPT_IGNORE) };
    int c = prompt_choice(data, i18n_get(STR_EV_PHONE_RINGS), opts, 2, 2);
    bool answered = (c == 1);

    if (answered) {
        draw_hud(data);
        narrative_beat(data, i18n_get(STR_EV_PHONE_STATIC));
        draw_hud(data);
        narrative_beat(data, i18n_get(STR_EV_PHONE_WAS_YOU));
    }
    resolve_choice(data, answered);
}

static void event_knock(GameData* data) {
    story_set_flag(&data->story, STORY_FLAG_EVENT_KNOCK);

    const char* opts[2] = { i18n_get(STR_OPT_OPEN), i18n_get(STR_OPT_DONT_OPEN) };
    int c = prompt_choice(data, i18n_get(STR_EV_KNOCK_PROMPT), opts, 2, 2);
    bool opened = (c == 1);

    draw_hud(data);
    narrative_beat(data, opened ? i18n_get(STR_EV_KNOCK_OPENED_NOBODY) : i18n_get(STR_EV_KNOCK_NOT_OPENED));
    resolve_choice(data, opened);
}

static void event_morgue_call(GameData* data) {
    story_set_flag(&data->story, STORY_FLAG_EVENT_MORGUE);

    draw_hud(data);
    announce_incoming_call(data);
    draw_hud(data);
    narrative_beat(data, i18n_get(STR_EV_MORGUE_CALL));

    const char* opts[2] = { i18n_get(STR_OPT_GO_NOW), i18n_get(STR_OPT_IGNORE_CALL) };
    int go = prompt_choice(data, i18n_get(STR_PATIENT_GO_PROMPT), opts, 2, 2);

    if (go == 1) {
        data->current_room = ROOM_MORGUE;
        draw_hud(data);
        narrative_beat(data, i18n_get(STR_EV_MORGUE_TEXT));
        apply_affliction_delta(data, 12);
        data->current_room = ROOM_MONITORING;
    } else {
        apply_affliction_delta(data, 8);
    }
}

/* ---- cenários de paciente (a mecânica central) --------------------------- */

/* Agenda a consequência tardia de negligência do cenário: o paciente que você
 * não tratou direito piora mais tarde, batendo mais forte do que qualquer
 * desfecho de ter ajudado. */
static void schedule_neglect(GameData* data, const PatientScenario* sc) {
    pending_consequences_add(&data->pending_consequences, sc->room_number,
                             sc->neglect_text, sc->delta_neglect,
                             data->game_time.total_minutes + sc->neglect_delay_min);
}

static void run_patient_scenario(GameData* data, const PatientScenario* sc) {
    /* Passo 1: o chamado chega (mostrado com o número do quarto, se houver). */
    draw_hud(data);
    draw_room_label(sc->room_number);
    announce_incoming_call(data);
    draw_hud(data);
    draw_room_label(sc->room_number);
    narrative_beat(data, i18n_get(sc->call_text));

    /* Passo 2: pergunta antes de mandar o jogador a algum lugar (nunca move em
     * silêncio). */
    const char* go_opts[2] = { i18n_get(STR_OPT_GO_NOW), i18n_get(STR_OPT_IGNORE_CALL) };
    int go = prompt_choice(data, i18n_get(STR_PATIENT_GO_PROMPT), go_opts, 2, 2);
    if (go == 2) {
        /* Ignorar é evasivo e tem um preço tardio proporcional à gravidade. O
         * jogador permanece onde está. */
        register_ignore(data);
        schedule_neglect(data, sc);
        return;
    }

    /* O jogador aceitou ir, então agora o movemos. */
    data->current_room = ROOM_PATIENT;
    data->ignore_streak = 0;

    /* Passo 3: a condição do paciente. */
    draw_hud(data);
    narrative_beat(data, i18n_get(sc->condition_text));

    /* Passo 4: uma decisão real de tratamento com três opções. */
    const char* topts[3] = {
        i18n_get(sc->options[0].label),
        i18n_get(sc->options[1].label),
        i18n_get(sc->options[2].label),
    };
    int choice = prompt_choice(data, i18n_get(STR_PATIENT_TREAT_PROMPT), topts, 3, 2);
    int idx = choice - 1;
    const TreatmentOption* opt = &sc->options[idx];

    bool complicated = false;
    if (idx == sc->administer_index && sc->complication_chance_den > 0) {
        complicated = (random_range(0, sc->complication_chance_den - 1) == 0);
    }

    if (complicated) {
        /* Passo 5: a intervenção dá errado -> chamar ajuda vs. sair na surdina. */
        draw_hud(data);
        narrative_beat(data, i18n_get(sc->complication_text));

        const char* sub_opts[2] = { i18n_get(STR_OPT_CALL_HELP), i18n_get(STR_OPT_SNEAK_OUT) };
        int sub = prompt_choice(data, NULL, sub_opts, 2, 1);

        draw_hud(data);
        if (sub == 1) {
            narrative_beat(data, i18n_get(STR_COMPLICATION_HELP_RESOLUTION));
            apply_affliction_delta(data, COMPLICATION_HELP_PENALTY);
        } else {
            narrative_beat(data, i18n_get(STR_COMPLICATION_SNEAK_RESOLUTION));
            apply_affliction_delta(data, COMPLICATION_SNEAK_PENALTY);
            register_ignore(data);   /* sair na surdina é o padrão evasivo */
        }
        /* Agir sobre o paciente é sempre adequado; sem morte tardia. */
    } else {
        draw_hud(data);
        narrative_beat(data, i18n_get(opt->outcome));
        apply_affliction_delta(data, opt->delta);
        if (!opt->adequate) {
            /* Passo 6: uma escolha inadequada maltrata o paciente -> tardia. */
            schedule_neglect(data, sc);
        }
    }

    data->current_room = ROOM_MONITORING;
}

/* Drena toda consequência de negligência cujo prazo venceu, uma cena para cada.
 * Retorna true se ao menos uma disparou (para o chamador pular uma frase
 * ambiente no mesmo tick). */
static bool check_pending_consequences(GameData* data) {
    bool any_fired = false;
    PendingConsequence due;

    while (pending_consequences_pop_due(&data->pending_consequences, data->game_time.total_minutes, &due)) {
        any_fired = true;

        draw_hud(data);
        draw_room_label(due.room_number);
        narrative_beat(data, i18n_get(due.consequence_text));
        apply_affliction_delta(data, due.delta);
    }

    return any_fired;
}

/* ---- frases ambientes de clima/aflição ----------------------------------- */

/* Cada frase ambiente é um texto + um custo de aflição. As de custo 0 são só
 * clima; as de custo > 0 (as últimas) também elevam a aflição ao aparecer, no
 * espírito de "Sua aflição aumenta...". Ampliar este conjunto é a forma mais
 * simples de espalhar mais tensão pelo turno sem depender só dos 20 pacientes. */
typedef struct {
    StringID text;
    int affliction; /* +aflição aplicada ao disparar (0 = só clima) */
} AmbientStinger;

static const AmbientStinger ambient_stingers[] = {
    { STR_STINGER_WHISPER,          0 },
    { STR_STINGER_FOOTSTEPS,        0 },
    { STR_STINGER_DISTANT_SCREAM,   0 },
    { STR_STINGER_WRONG_SILENCE,    0 },
    { STR_STINGER_COLD_DRAFT,       0 },
    { STR_STINGER_FLICKER,          0 },
    { STR_STINGER_WALLS_SPEAK,      0 },
    { STR_STINGER_MORGUE_COLD,      0 },
    { STR_STINGER_BREATH_NECK,      0 },
    { STR_STINGER_NAME_CALLED,      0 },
    { STR_STINGER_SHADOW_DOOR,      0 },
    { STR_STINGER_EMPTY_BED,        0 },
    { STR_STINGER_AFFLICTION_RISES, 2 },
    { STR_STINGER_DREAD_GROWS,      2 },
    { STR_STINGER_NOT_ALONE,        1 },
};
#define AMBIENT_STINGER_COUNT (int)(sizeof(ambient_stingers) / sizeof(ambient_stingers[0]))

/* Avaliada no máximo uma vez por minuto exibido (ver gameplay_loop). A chance
 * cresce quanto mais o turno avança e quanto maior a aflição, com um piso para
 * nunca virar quase-certeza a cada minuto. Retorna true se uma frase apareceu
 * (o chamador precisa saber, pois a cena consome tempo real). */
static bool maybe_fire_ambient_stinger(GameData* data, int* last_stinger_minute) {
    int t = data->game_time.total_minutes;
    if (t < T_AMBIENT_STINGER_START) {
        return false;
    }
    if (*last_stinger_minute >= 0 && t - *last_stinger_minute < STINGER_MIN_COOLDOWN_MIN) {
        return false;
    }

    int minutes_elapsed = t - T_AMBIENT_STINGER_START;
    int affliction = get_affliction_percent(data);
    int chance_den = STINGER_BASE_CHANCE_DEN - (minutes_elapsed / 15) - (affliction / 8);
    if (chance_den < 6) {
        chance_den = 6;
    }

    if (random_range(0, chance_den - 1) != 0) {
        return false;
    }

    *last_stinger_minute = t;

    /* Sorteia uma frase; se ela tiver custo, aplica a aflição antes de mostrar,
     * para o HUD já refletir o novo valor. */
    const AmbientStinger* pick = &ambient_stingers[random_range(0, AMBIENT_STINGER_COUNT - 1)];
    if (pick->affliction > 0) {
        apply_affliction_delta(data, pick->affliction);
    }
    draw_hud(data);
    narrative_beat(data, i18n_get(pick->text));
    return true;
}

/* ---- agendamento de eventos ---------------------------------------------- */

/* Depois de qualquer evento interativo, reserva uma janela silenciosa aleatória
 * antes que o próximo possa disparar. Isto sempre vence o horário agendado de
 * um evento, evitando que os chamados se empilhem. */
static void bump_event_gap(GameData* data) {
    data->next_event_available_at = data->game_time.total_minutes
        + random_range(EVENT_GAP_MIN, EVENT_GAP_MAX);
}

/* Retorna true se algo disparou nesta chamada (usado para pular uma frase
 * ambiente no mesmo tick, de modo que cenas roteirizadas nunca sejam cortadas).
 *
 * Garantias de término / não-encadeamento:
 *  - Todo evento marca sua flag de "já disparou" / avança seu contador ANTES de
 *    o loop poder testar a mesma condição de novo, então nada dispara duas vezes.
 *  - Cada evento disparado empurra next_event_available_at para um ponto no
 *    futuro, e o gate no topo do loop sai assim que t < esse ponto. Como uma
 *    cena só avança o relógio, o gate é sempre acionado logo após um evento
 *    rodar, então NO MÁXIMO UM evento dispara por chamada — sem pilhas, e o
 *    loop sempre termina. */
static bool check_and_fire_events(GameData* data) {
    bool any_fired = false;
    bool fired = true;

    while (fired) {
        fired = false;
        int t = data->game_time.total_minutes;

        /* Janela silenciosa obrigatória: nada novo é sequer considerado antes de
         * passar dela. */
        if (t < data->next_event_available_at) {
            break;
        }

        if (t >= T_MONITOR_FLICKER && !story_has_flag(&data->story, STORY_FLAG_EVENT_MONITOR)) {
            event_monitor_flicker(data);
            fired = true;
            any_fired = true;
            continue;
        }
        if (t >= T_RECEPTION_CALL && !story_has_flag(&data->story, STORY_FLAG_EVENT_RECEPTION)) {
            event_reception_call(data);
            bump_event_gap(data);
            fired = true;
            any_fired = true;
            continue;
        }
        if (t >= T_PHONE_SELF && !story_has_flag(&data->story, STORY_FLAG_EVENT_PHONE_SELF)) {
            event_phone_self(data);
            bump_event_gap(data);
            fired = true;
            any_fired = true;
            continue;
        }
        if (t >= T_KNOCK && !story_has_flag(&data->story, STORY_FLAG_EVENT_KNOCK)) {
            event_knock(data);
            bump_event_gap(data);
            fired = true;
            any_fired = true;
            continue;
        }
        if (t >= T_MORGUE_CALL && !story_has_flag(&data->story, STORY_FLAG_EVENT_MORGUE)) {
            event_morgue_call(data);
            bump_event_gap(data);
            fired = true;
            any_fired = true;
            continue;
        }
        if (data->patient_scenarios_fired < patient_scenario_count()) {
            const PatientScenario* sc = patient_scenario_get(data->patient_scenarios_fired);
            if (sc != NULL && t >= sc->call_minute) {
                /* Avança o contador ANTES de rodar para que o mesmo cenário
                 * nunca seja reentrado, mesmo no meio da cena. */
                data->patient_scenarios_fired++;
                run_patient_scenario(data, sc);
                bump_event_gap(data);
                fired = true;
                any_fired = true;
                continue;
            }
        }
    }

    return any_fired;
}

/* ---- finais -------------------------------------------------------------- */

/* Flash cru, brusco, impulável e sem tick — não é um narrative_beat. O jogo
 * nunca confirma o que foi. */
static void flash_ward_307(void) {
    renderer_clear();
    printf("\n\n\n\n");
    ui_center_line(i18n_get(STR_WARD_307_FLASH));
    fflush(stdout);
    sleep_ms(1400);
    renderer_clear();
}

static GameplayResult run_win_ending(GameData* data) {
    flash_ward_307();

    renderer_clear();
    printf("\n\n");
    ui_center_line(i18n_get(STR_WIN_TITLE));
    printf("\n\n");
    ui_center_line(i18n_get(STR_WIN_TEXT_1));
    narrative_beat(data, i18n_get(STR_WIN_TEXT_2));
    return GAMEPLAY_RESULT_ENDED;
}

static GameplayResult run_cardiac_ending(GameData* data) {
    renderer_clear();
    printf("\n\n\n");
    narrative_beat(data, i18n_get(STR_CARDIAC_TEXT));

    renderer_clear();
    printf("\n\n\n");
    narrative_beat(data, i18n_get(STR_CARDIAC_WARD_LINE));

    return GAMEPLAY_RESULT_ENDED;
}

/* Retorna true se um final de verdade foi alcançado (resultado escrito em *out).
 * Retorna false se o jogador negou e o turno recomeça. */
static bool run_faint_sequence(GameData* data, GameplayResult* out) {
    renderer_clear();
    printf("\n\n");
    narrative_beat(data, i18n_get(STR_FAINT_TEXT_1));

    renderer_clear();
    printf("\n\n");
    ui_center_line(i18n_get(STR_FAINT_TEXT_2));
    printf("\n\n");
    ui_center_line(i18n_get(STR_FAINT_QUESTION));
    printf("\n\n");
    ui_center_line(i18n_get(STR_FAINT_ACCEPT));
    printf("\n");
    ui_center_line(i18n_get(STR_FAINT_DENY));
    printf("\n\n");
    fflush(stdout);

    InputKey key;
    while (1) {
        key = input_get_key();
        if (key == INPUT_1 || key == INPUT_2 || key == INPUT_ESCAPE) {
            if (key == INPUT_ESCAPE) key = INPUT_2;
            break;
        }
        renderer_clear();
        printf("\n");
        ui_center_line(i18n_get(STR_MSG_INVALID_OPTION_RETRY));
        printf("\n\n");
        ui_center_line(i18n_get(STR_FAINT_ACCEPT));
        printf("\n");
        ui_center_line(i18n_get(STR_FAINT_DENY));
        printf("\n\n");
        fflush(stdout);
    }

    if (key == INPUT_1) {
        renderer_clear();
        printf("\n\n");
        narrative_beat(data, i18n_get(STR_FAINT_ACCEPT_TEXT));

        renderer_clear();
        printf("\n\n");
        narrative_beat(data, i18n_get(STR_FAINT_ACCEPT_WARD_REVEAL));

        *out = GAMEPLAY_RESULT_ENDED;
        return true;
    }

    /* Negar: reinicia o turno, mantém o mesmo personagem e marca que isto já
     * aconteceu antes, para a próxima run trazer uma dica sutil. */
    renderer_clear();
    printf("\n\n");
    narrative_beat(data, i18n_get(STR_FAINT_DENY_TEXT));

    data->story.flags = 0;
    story_set_flag(&data->story, STORY_FLAG_DENIED_ONCE);

    time_init(&data->game_time);
    data->current_room = ROOM_MONITORING;
    data->denial_index = 0;
    data->ignore_streak = 0;
    data->patient_scenarios_fired = 0;
    data->next_event_available_at = 0;
    data->last_ignore_minute = 0;
    pending_consequences_init(&data->pending_consequences);
    data->player.sanity = data->player.max_sanity;

    run_intro(data);

    if (story_has_flag(&data->story, STORY_FLAG_DENIED_ONCE)) {
        renderer_clear();
        printf("\n");
        narrative_beat(data, i18n_get(STR_DENIED_BEFORE_HINT));
    }

    return false;
}

/* ---- salvar e sair ------------------------------------------------------- */

/* Salva a partida no meio do turno (tecla "0"): grava o save.txt e mostra
 * "Salvando..." rapidamente, sem interromper o plantão. Bem diferente do antigo
 * "sair e perder o progresso": aqui o jogo continua logo depois. */
static void save_during_shift(GameData* data) {
    save_game(data);
    renderer_clear();
    printf("\n");
    ui_center_line(i18n_get(STR_SAVE_SAVING));
    printf("\n");
    renderer_refresh();
    sleep_ms(700);
}

/* Menu de pausa, aberto com Esc. O relógio fica parado enquanto ele está na
 * tela (prompt_choice é bloqueante). Retorna true se o jogador decidiu sair do
 * plantão, false se decidiu continuar. */
static bool pause_menu(GameData* data) {
    const char* opts[4] = {
        i18n_get(STR_PAUSE_CONTINUE),
        i18n_get(STR_PAUSE_SAVE),
        i18n_get(STR_PAUSE_SAVE_QUIT),
        i18n_get(STR_PAUSE_QUIT),
    };
    /* Esc/EOF cai na opção 1 (continuar), então apertar Esc de novo fecha o
     * menu e nenhum stdin fechado prende o jogo aqui. */
    int choice = prompt_choice(data, i18n_get(STR_PAUSE_TITLE), opts, 4, 1);

    if (choice == 2 || choice == 3) {
        save_during_shift(data);
    }

    if (choice == 1 || choice == 2) {
        return false;   /* continua o plantão */
    }

    /* Sair (com ou sem ter salvado antes). */
    renderer_clear();
    printf("\n");
    narrative_beat(data, i18n_get(STR_QUIT_CONFIRMED));
    return true;
}

/* ---- loop principal em tempo real ---------------------------------------- */

static GameplayResult gameplay_loop(GameData* data) {
    long long last_tick = get_time_ms();
    long long minute_acc_ms = 0;
    int last_drawn_minute = -1;
    int last_stinger_minute = -1;

    draw_hud(data);

    while (1) {
        sleep_ms(80);
        long long now = get_time_ms();
        long long delta = now - last_tick;
        last_tick = now;
        minute_acc_ms += delta;

        /* Ritmo único 1:1: a cada segundo real acumulado avança 1 minuto de
         * jogo e rola a chance do gotejar de aflição de fundo. Enquanto uma
         * decisão está na tela o loop nem roda, então o relógio fica parado. */
        while (minute_acc_ms >= REAL_MS_PER_GAME_MINUTE) {
            minute_acc_ms -= REAL_MS_PER_GAME_MINUTE;
            game_data_advance_time(data, 1);
            if (random_range(0, 6) == 0) {
                apply_affliction_delta(data, 1);
            }
        }

        if (data->game_time.total_minutes >= WIN_MINUTES) {
            return run_win_ending(data);
        }
        if (get_affliction_percent(data) >= 100) {
            return run_cardiac_ending(data);
        }
        if (data->ignore_streak >= FAINT_STREAK_THRESHOLD) {
            GameplayResult result;
            if (run_faint_sequence(data, &result)) {
                return result;
            }
            last_drawn_minute = -1;
            last_stinger_minute = -1;
            last_tick = get_time_ms();
            minute_acc_ms = 0;
            continue;
        }

        /* Esc apertado durante uma cena narrativa (a cena consumiu a tecla):
         * abre o menu de pausa agora, para o Esc nunca se perder. */
        if (pause_requested) {
            pause_requested = false;
            if (pause_menu(data)) {
                return GAMEPLAY_RESULT_ABANDONED;
            }
            last_drawn_minute = -1;
            last_tick = get_time_ms();
            minute_acc_ms = 0;
            continue;
        }

        bool events_fired = check_and_fire_events(data);
        bool consequences_fired = check_pending_consequences(data);

        bool minute_changed = (data->game_time.total_minutes != last_drawn_minute);
        bool stinger_fired = false;

        if (minute_changed && !events_fired && !consequences_fired) {
            stinger_fired = maybe_fire_ambient_stinger(data, &last_stinger_minute);
        }

        if (minute_changed) {
            /* Cura um ponto do streak de evasão depois de um trecho sem nenhuma
             * nova escolha evasiva, para que um azar inicial não condene a run. */
            if (data->ignore_streak > 0 &&
                data->game_time.total_minutes - data->last_ignore_minute >= IGNORE_DECAY_MINUTES) {
                data->ignore_streak--;
                data->last_ignore_minute = data->game_time.total_minutes;
            }
            draw_hud(data);
            last_drawn_minute = data->game_time.total_minutes;
        }

        /* Teclas fora de um chamado: "0" salva na hora; Esc abre o menu de
         * pausa. */
        bool key_scene_ran = false;
        if (input_has_input()) {
            InputKey key = input_get_key();
            if (key == INPUT_0) {
                save_during_shift(data);
                last_drawn_minute = -1;
                key_scene_ran = true;
            } else if (key == INPUT_ESCAPE) {
                if (pause_menu(data)) {
                    return GAMEPLAY_RESULT_ABANDONED;
                }
                last_drawn_minute = -1;
                key_scene_ran = true;
            }
        }

        /* Qualquer cena/menu acima consumiu tempo REAL enquanto o relógio do
         * jogo estava parado (ou já avançou o relógio por conta própria, no caso
         * das cenas narrativas). Reancorar a base de medição aqui descarta esse
         * tempo real, senão o próximo tick converteria toda a duração da
         * decisão em minutos de jogo de uma vez — o relógio "pulava" logo depois
         * de o jogador escolher uma opção. */
        if (events_fired || consequences_fired || stinger_fired || key_scene_ran) {
            last_tick = get_time_ms();
            minute_acc_ms = 0;
        }
    }
}

/* ---- pontos de entrada públicos ------------------------------------------ */

GameplayResult gameplay_start_new(GameData* data) {
    run_guide();
    run_character_creation(data);

    data->current_room = ROOM_MONITORING;

    run_intro(data);
    run_opening(data);

    return gameplay_loop(data);
}

/* Retoma um plantão salvo: lê o save.txt para `data` e cai direto no loop, sem
 * criação de personagem nem intro. Se o save não puder ser lido, devolve
 * ABANDONED para o chamador voltar ao menu sem começar nada. */
GameplayResult gameplay_start_loaded(GameData* data) {
    if (!load_game(data)) {
        return GAMEPLAY_RESULT_ABANDONED;
    }
    return gameplay_loop(data);
}
