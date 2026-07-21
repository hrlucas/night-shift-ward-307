#include "ui/box.h"
#include "ui/renderer.h"
#include <stdio.h>
#include <string.h>

/* Largura visível de uma string UTF-8: conta só os bytes que INICIAM um
 * caractere (ou seja, ignora os bytes de continuação, cujos dois bits mais
 * altos são "10"). strlen() conta bytes, então qualquer texto em PT com acento
 * (ç, ã, é, …) seria contado a mais e desalinharia a centralização/margem da
 * caixa. */
static int utf8_visible_width(const char* text) {
    int width = 0;
    for (const unsigned char* p = (const unsigned char*)text; *p != '\0'; p++) {
        if ((*p & 0xC0) != 0x80) {
            width++;
        }
    }
    return width;
}

/* Número de bytes de `text` que formam os primeiros `max_visible` caracteres
 * visíveis, nunca cortando um caractere multibyte no meio. Usado para limitar
 * texto longo demais ao interior da caixa sem gerar mojibake. */
static int utf8_prefix_bytes(const char* text, int max_visible) {
    int width = 0;
    int i = 0;
    while (text[i] != '\0') {
        if (((unsigned char)text[i] & 0xC0) != 0x80) {
            if (width >= max_visible) {
                break;
            }
            width++;
        }
        i++;
    }
    return i;
}

void ui_print_spaces(int n) {
    for (int i = 0; i < n; i++) {
        putchar(' ');
    }
}

int ui_box_margin(int box_width) {
    int term_width = renderer_get_width();
    int margin = (term_width - box_width) / 2;
    return (margin > 0) ? margin : 0;
}

void ui_box_top(int width) {
    int margin = ui_box_margin(width);
    ui_print_spaces(margin);
    printf("\xE2\x95\x94"); /* ╔ */
    for (int i = 0; i < width - 2; i++) {
        printf("\xE2\x95\x90"); /* ═ */
    }
    printf("\xE2\x95\x97\n"); /* ╗ */
}

void ui_box_divider(int width) {
    int margin = ui_box_margin(width);
    ui_print_spaces(margin);
    printf("\xE2\x95\xA0"); /* ╠ */
    for (int i = 0; i < width - 2; i++) {
        printf("\xE2\x95\x90"); /* ═ */
    }
    printf("\xE2\x95\xA3\n"); /* ╣ */
}

void ui_box_bottom(int width) {
    int margin = ui_box_margin(width);
    ui_print_spaces(margin);
    printf("\xE2\x95\x9A"); /* ╚ */
    for (int i = 0; i < width - 2; i++) {
        printf("\xE2\x95\x90"); /* ═ */
    }
    printf("\xE2\x95\x9D\n"); /* ╝ */
}

void ui_box_empty(int width) {
    int margin = ui_box_margin(width);
    ui_print_spaces(margin);
    printf("\xE2\x95\x91"); /* ║ */
    ui_print_spaces(width - 2);
    printf("\xE2\x95\x91\n"); /* ║ */
}

void ui_box_text(int width, const char* text) {
    int margin = ui_box_margin(width);
    int inner = width - 2;
    int vis = utf8_visible_width(text);
    int bytes = (int)strlen(text);
    if (vis > inner) {
        bytes = utf8_prefix_bytes(text, inner);
        vis = inner;
    }
    int pad_left = (inner - vis) / 2;
    int pad_right = inner - vis - pad_left;

    ui_print_spaces(margin);
    printf("\xE2\x95\x91"); /* ║ */
    ui_print_spaces(pad_left);
    printf("%.*s", bytes, text);
    ui_print_spaces(pad_right);
    printf("\xE2\x95\x91\n"); /* ║ */
}

void ui_box_text_left(int width, const char* text) {
    int margin = ui_box_margin(width);
    int inner = width - 2;
    int vis = utf8_visible_width(text);
    int bytes = (int)strlen(text);
    if (vis > inner - 2) {
        bytes = utf8_prefix_bytes(text, inner - 2);
        vis = inner - 2;
    }
    int pad_right = inner - vis - 2;

    ui_print_spaces(margin);
    printf("\xE2\x95\x91  "); /* ║  */
    printf("%.*s", bytes, text);
    ui_print_spaces(pad_right);
    printf("\xE2\x95\x91\n"); /* ║ */
}

void ui_center_line(const char* text) {
    int width = renderer_get_width();
    int vis = utf8_visible_width(text);
    int pad = (width - vis) / 2;
    if (pad < 0) {
        pad = 0;
    }
    ui_print_spaces(pad);
    printf("%s", text);
}
