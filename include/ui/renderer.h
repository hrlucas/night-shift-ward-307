#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include "core/utils.h"

/**
 * @file renderer.h
 * @brief Sistema de renderização ASCII/ANSI e setup do terminal.
 */

/**
 * @brief Códigos de cor para o terminal.
 */
typedef enum {
    COLOR_RESET,
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    COLOR_BRIGHT_BLACK,
    COLOR_BRIGHT_RED,
    COLOR_BRIGHT_GREEN,
    COLOR_BRIGHT_YELLOW,
    COLOR_BRIGHT_BLUE,
    COLOR_BRIGHT_MAGENTA,
    COLOR_BRIGHT_CYAN,
    COLOR_BRIGHT_WHITE
} Color;

/**
 * @brief Inicializa o renderer (UTF-8 + ANSI no console do Windows).
 */
void renderer_init(void);

/**
 * @brief Finaliza o renderer.
 */
void renderer_shutdown(void);

/**
 * @brief Limpa a tela.
 */
void renderer_clear(void);

/**
 * @brief Define a cor do texto.
 */
void renderer_set_color(Color color);

/**
 * @brief Restaura a cor padrão.
 */
void renderer_reset_color(void);

/**
 * @brief Desenha texto em uma posição.
 */
void renderer_draw_text(int x, int y, const char* text);

/**
 * @brief Desenha texto centralizado.
 */
void renderer_draw_centered(int y, const char* text);

/**
 * @brief Desenha uma caixa/borda.
 */
void renderer_draw_box(int x, int y, int width, int height);

/**
 * @brief Desenha uma linha horizontal.
 */
void renderer_draw_hline(int x, int y, int length);

/**
 * @brief Desenha uma linha vertical.
 */
void renderer_draw_vline(int x, int y, int length);

/**
 * @brief Desenha uma barra de progresso.
 */
void renderer_draw_progress_bar(int x, int y, int width, int percent, Color color);

/**
 * @brief Verifica se o terminal suporta cor.
 */
bool renderer_supports_color(void);

/**
 * @brief Verifica se o terminal suporta unicode.
 */
bool renderer_supports_unicode(void);

/**
 * @brief Retorna a largura do terminal.
 */
int renderer_get_width(void);

/**
 * @brief Retorna a altura do terminal.
 */
int renderer_get_height(void);

/**
 * @brief Atualiza a exibição.
 */
void renderer_refresh(void);

#endif /* RENDERER_H */
