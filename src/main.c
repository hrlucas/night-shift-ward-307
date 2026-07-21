#include "core/game.h"
#include "core/state.h"
#include "core/i18n.h"
#include "core/utils.h"
#include "ui/input.h"
#include "ui/renderer.h"
#include "ui/box.h"
#include "data/game_state.h"
#include "systems/gameplay.h"
#include "save/save.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Callbacks de estado ------------------------------------------------- */

static void on_boot_enter(void);
static void on_boot_exit(void);
static void on_boot_update(float delta_time);
static void on_boot_render(void);
static void on_boot_input(int input);

static void on_title_enter(void);
static void on_title_exit(void);
static void on_title_update(float delta_time);
static void on_title_render(void);
static void on_title_input(int input);

static void draw_main_menu(void);
static void on_main_menu_enter(void);
static void on_main_menu_exit(void);
static void on_main_menu_update(float delta_time);
static void on_main_menu_render(void);
static void on_main_menu_input(int input);

static void on_new_game_enter(void);
static void on_new_game_exit(void);
static void on_new_game_update(float delta_time);
static void on_new_game_render(void);
static void on_new_game_input(int input);

static void on_shutdown_enter(void);
static void on_shutdown_render(void);

/* Quando o jogador escolhe "Carregar Jogo" no menu, marcamos esta flag antes de
 * entrar em STATE_PLAY. on_new_game_enter a lê para decidir entre começar um
 * plantão novo ou retomar o save. É a forma mais simples de reaproveitar o
 * mesmo estado de jogo para os dois caminhos. */
static bool load_requested = false;

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (!game_init()) {
        fprintf(stderr, "Failed to initialize game\n");
        return 1;
    }

    input_init();
    renderer_init();

    /* Registra cada estado com seus callbacks (on_enter/exit/update/render/
     * input). A máquina de estados vive em core/state.c. */
    StateDescriptor boot_state = {
        STATE_BOOT,
        on_boot_enter,
        on_boot_exit,
        on_boot_update,
        on_boot_render,
        on_boot_input
    };
    state_register(STATE_BOOT, boot_state);

    StateDescriptor title_state = {
        STATE_TITLE_SCREEN,
        on_title_enter,
        on_title_exit,
        on_title_update,
        on_title_render,
        on_title_input
    };
    state_register(STATE_TITLE_SCREEN, title_state);

    StateDescriptor main_menu_state = {
        STATE_MAIN_MENU,
        on_main_menu_enter,
        on_main_menu_exit,
        on_main_menu_update,
        on_main_menu_render,
        on_main_menu_input
    };
    state_register(STATE_MAIN_MENU, main_menu_state);

    StateDescriptor play_state = {
        STATE_PLAY,
        on_new_game_enter,
        on_new_game_exit,
        on_new_game_update,
        on_new_game_render,
        on_new_game_input
    };
    state_register(STATE_PLAY, play_state);

    StateDescriptor shutdown_state = {
        STATE_SHUTDOWN,
        on_shutdown_enter,
        NULL,
        NULL,
        on_shutdown_render,
        NULL
    };
    state_register(STATE_SHUTDOWN, shutdown_state);

    state_transition(STATE_BOOT);

    game_run();

    input_shutdown();
    renderer_shutdown();
    game_shutdown();

    return 0;
}

/* ---- Estado de boot ------------------------------------------------------ */

static void on_boot_enter(void) {
    renderer_clear();
    printf("%s\n", i18n_get(STR_BOOT_INIT));
    renderer_refresh();

    for (int i = 0; i < 2; i++) {
        sleep_ms(400);
        printf(".\n");
        renderer_refresh();
    }

    sleep_ms(400);
    printf("%s\n", i18n_get(STR_BOOT_DONE));
    renderer_refresh();
    sleep_ms(500);

    /* Avança sozinho: o boot nunca espera por tecla, então nenhuma tecla real
     * digitada pelo jogador é consumida em silêncio aqui. */
    state_transition(STATE_TITLE_SCREEN);
}

static void on_boot_exit(void) {

}

static void on_boot_update(float delta_time) {
    (void)delta_time;
}

static void on_boot_render(void) {

}

static void on_boot_input(int input) {
    (void)input;
}

/* ---- Tela de título ------------------------------------------------------ */

static void on_title_enter(void) {
    renderer_clear();
    printf("\n");
    ui_box_top(UI_BOX_WIDTH);
    ui_box_empty(UI_BOX_WIDTH);
    ui_box_text(UI_BOX_WIDTH, "GRAVEYARD SHIFT");
    ui_box_empty(UI_BOX_WIDTH);
    ui_box_text(UI_BOX_WIDTH, i18n_get(STR_PRESS_ANY_KEY));
    ui_box_empty(UI_BOX_WIDTH);
    ui_box_bottom(UI_BOX_WIDTH);
    renderer_refresh();
}

static void on_title_exit(void) {

}

static void on_title_update(float delta_time) {
    (void)delta_time;
}

static void on_title_render(void) {

}

static void on_title_input(int input) {
    (void)input;
    state_transition(STATE_MAIN_MENU);
}

/* ---- Menu principal ------------------------------------------------------ */

/* Desenha a caixa do menu. Os itens já trazem o número da tecla embutido no
 * próprio texto em i18n.c. "Carregar Jogo" só aparece quando existe um save no
 * disco; sem save o menu tem apenas dois itens e "Sair" passa a ser a tecla 2. */
static void draw_main_menu(void) {
    bool has_save = save_exists();

    printf("\n");
    ui_box_top(UI_BOX_WIDTH);
    ui_box_text(UI_BOX_WIDTH, i18n_get(STR_MENU_TITLE));
    ui_box_divider(UI_BOX_WIDTH);
    ui_box_empty(UI_BOX_WIDTH);
    ui_box_text_left(UI_BOX_WIDTH, i18n_get(STR_MENU_NEW_GAME));
    if (has_save) {
        ui_box_text_left(UI_BOX_WIDTH, i18n_get(STR_MENU_LOAD_GAME));
        ui_box_text_left(UI_BOX_WIDTH, i18n_get(STR_MENU_QUIT));
    } else {
        ui_box_text_left(UI_BOX_WIDTH, i18n_get(STR_MENU_QUIT_ALT));
    }
    ui_box_empty(UI_BOX_WIDTH);
    ui_box_bottom(UI_BOX_WIDTH);
    printf("\n");
    ui_center_line(i18n_get(STR_SELECT_OPTION));
    renderer_refresh();
}

static void on_main_menu_enter(void) {
    renderer_clear();
    draw_main_menu();
}

static void on_main_menu_exit(void) {

}

static void on_main_menu_update(float delta_time) {
    (void)delta_time;
}

static void on_main_menu_render(void) {

}

/* O roteamento acompanha o menu desenhado: com save, 1=Jogar, 2=Carregar,
 * 3=Sair; sem save, 1=Jogar e 2=Sair (a tecla 3 não existe). */
static void on_main_menu_input(int input) {
    bool has_save = save_exists();

    switch (input) {
        case INPUT_1:
            printf("%s\n", i18n_get(STR_MSG_STARTING_NEW_GAME));
            state_transition(STATE_PLAY);
            return;
        case INPUT_2:
            if (has_save) {
                printf("%s\n", i18n_get(STR_MSG_LOADING_GAME));
                load_requested = true;
                state_transition(STATE_PLAY);
            } else {
                printf("%s\n", i18n_get(STR_MSG_QUITTING));
                state_transition(STATE_SHUTDOWN);
            }
            return;
        case INPUT_3:
            if (has_save) {
                printf("%s\n", i18n_get(STR_MSG_QUITTING));
                state_transition(STATE_SHUTDOWN);
                return;
            }
            break;   /* sem save não há opção 3: cai no aviso de inválida */
        case INPUT_Q:
        case INPUT_ESCAPE:
            printf("%s\n", i18n_get(STR_MSG_QUITTING));
            state_transition(STATE_SHUTDOWN);
            return;
        default:
            break;
    }

    renderer_clear();
    printf("\n");
    ui_center_line(i18n_get(STR_MSG_INVALID_OPTION_RETRY));
    printf("\n");
    draw_main_menu();
}

/* ---- Jogo ---------------------------------------------------------------- */

/* Toda a sessão (criação de personagem + turno em tempo real, ou a retomada de
 * um save) roda sincronamente aqui dentro. Ao terminar, decidimos o que fazer
 * com o arquivo de save e voltamos ao menu. */
static void on_new_game_enter(void) {
    GameData* data = game_get_data();
    GameplayResult result;

    game_reset();

    if (load_requested) {
        load_requested = false;
        result = gameplay_start_loaded(data);
    } else {
        result = gameplay_start_new(data);
    }

    /* Um plantão que chegou a um dos três finais não pode ser recarregado:
     * apagamos o save. Abandonar com Esc NÃO é um final, então ali o save é
     * preservado (dá para retomar depois pelo menu "Carregar Jogo"). */
    if (result == GAMEPLAY_RESULT_ENDED) {
        save_delete();
    }

    state_transition(STATE_MAIN_MENU);
}

static void on_new_game_exit(void) {

}

static void on_new_game_update(float delta_time) {
    (void)delta_time;
}

static void on_new_game_render(void) {

}

static void on_new_game_input(int input) {
    (void)input;
    state_transition(STATE_MAIN_MENU);
}

/* ---- Encerramento -------------------------------------------------------- */

static void on_shutdown_enter(void) {
    renderer_clear();
    printf("\n%s\n", i18n_get(STR_THANK_YOU));
}

static void on_shutdown_render(void) {

}
