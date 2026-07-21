#ifndef UI_BOX_H
#define UI_BOX_H

/**
 * @file box.h
 * @brief Helpers de desenho de caixas centralizadas, usados em várias telas.
 *        A largura é calculada contando caracteres visíveis (ciente de UTF-8),
 *        não bytes — essencial porque quase todo texto em PT tem acento.
 */

#define UI_BOX_WIDTH 70

void ui_print_spaces(int n);
int ui_box_margin(int box_width);
void ui_box_top(int width);
void ui_box_divider(int width);
void ui_box_bottom(int width);
void ui_box_empty(int width);
void ui_box_text(int width, const char* text);
void ui_box_text_left(int width, const char* text);
void ui_center_line(const char* text);

#endif /* UI_BOX_H */
