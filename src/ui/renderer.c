#include "ui/renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

static bool supports_color = true;
static bool supports_unicode = true;
static int terminal_width = 80;
static int terminal_height = 24;

void renderer_init(void) {
#ifdef _WIN32
    /* Força UTF-8 para os caracteres de caixa e o texto acentuado renderizarem
     * certo, em vez de virarem mojibake pela codepage OEM antiga do Windows. */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    supports_color = GetConsoleMode(hConsole, &mode);
    if (supports_color) {
        SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    supports_unicode = true;

    /* Detecta o tamanho do terminal; se a chamada falhar, mantém 80x24 (os
     * valores padrão), nunca lixo não inicializado. */
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        terminal_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        terminal_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    char* term = getenv("TERM");
    supports_color = (term != NULL && strstr(term, "color") != NULL);
    supports_unicode = true;
    
    /* Idem no POSIX: só usa o tamanho detectado se ioctl retornar OK e valores
     * válidos; senão cai no padrão 80x24. */
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0 && w.ws_row > 0) {
        terminal_width = w.ws_col;
        terminal_height = w.ws_row;
    } else {
        terminal_width = 80;
        terminal_height = 24;
    }
#endif
}

/* renderer_init prepara o terminal (UTF-8, ANSI, tamanho); a partir daqui as
 * funções abaixo apenas emitem texto/escapes ANSI. */

void renderer_shutdown(void) {
    supports_color = false;
    supports_unicode = false;
}

void renderer_clear(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void renderer_set_color(Color color) {
    if (!supports_color) return;
    
    switch (color) {
        case COLOR_RESET:
            printf("\033[0m");
            break;
        case COLOR_BLACK:
            printf("\033[30m");
            break;
        case COLOR_RED:
            printf("\033[31m");
            break;
        case COLOR_GREEN:
            printf("\033[32m");
            break;
        case COLOR_YELLOW:
            printf("\033[33m");
            break;
        case COLOR_BLUE:
            printf("\033[34m");
            break;
        case COLOR_MAGENTA:
            printf("\033[35m");
            break;
        case COLOR_CYAN:
            printf("\033[36m");
            break;
        case COLOR_WHITE:
            printf("\033[37m");
            break;
        case COLOR_BRIGHT_BLACK:
            printf("\033[90m");
            break;
        case COLOR_BRIGHT_RED:
            printf("\033[91m");
            break;
        case COLOR_BRIGHT_GREEN:
            printf("\033[92m");
            break;
        case COLOR_BRIGHT_YELLOW:
            printf("\033[93m");
            break;
        case COLOR_BRIGHT_BLUE:
            printf("\033[94m");
            break;
        case COLOR_BRIGHT_MAGENTA:
            printf("\033[95m");
            break;
        case COLOR_BRIGHT_CYAN:
            printf("\033[96m");
            break;
        case COLOR_BRIGHT_WHITE:
            printf("\033[97m");
            break;
    }
}

void renderer_reset_color(void) {
    if (supports_color) {
        printf("\033[0m");
    }
}

void renderer_draw_text(int x, int y, const char* text) {
    if (text == NULL) return;
    
    for (int i = 0; i < y; i++) {
        printf("\n");
    }
    
    for (int i = 0; i < x; i++) {
        printf(" ");
    }
    
    printf("%s", text);
}

void renderer_draw_centered(int y, const char* text) {
    if (text == NULL) return;
    
    int len = 0;
    while (text[len] != '\0') len++;
    
    int padding = (terminal_width - len) / 2;
    if (padding < 0) padding = 0;
    
    for (int i = 0; i < y; i++) {
        printf("\n");
    }
    
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    
    printf("%s", text);
}

void renderer_draw_box(int x, int y, int width, int height) {
    if (supports_unicode) {
        for (int i = 0; i < y; i++) {
            printf("\n");
        }
        
        for (int i = 0; i < x; i++) {
            printf(" ");
        }
        
        printf("┌");
        for (int i = 0; i < width - 2; i++) {
            printf("─");
        }
        printf("┐\n");
        
        for (int row = 0; row < height - 2; row++) {
            for (int i = 0; i < x; i++) {
                printf(" ");
            }
            printf("│");
            for (int i = 0; i < width - 2; i++) {
                printf(" ");
            }
            printf("│\n");
        }
        
        for (int i = 0; i < x; i++) {
            printf(" ");
        }
        
        printf("└");
        for (int i = 0; i < width - 2; i++) {
            printf("─");
        }
        printf("┘");
    } else {
        for (int i = 0; i < y; i++) {
            printf("\n");
        }
        
        for (int i = 0; i < x; i++) {
            printf(" ");
        }
        
        printf("+");
        for (int i = 0; i < width - 2; i++) {
            printf("-");
        }
        printf("+\n");
        
        for (int row = 0; row < height - 2; row++) {
            for (int i = 0; i < x; i++) {
                printf(" ");
            }
            printf("|");
            for (int i = 0; i < width - 2; i++) {
                printf(" ");
            }
            printf("|\n");
        }
        
        for (int i = 0; i < x; i++) {
            printf(" ");
        }
        
        printf("+");
        for (int i = 0; i < width - 2; i++) {
            printf("-");
        }
        printf("+");
    }
}

void renderer_draw_hline(int x, int y, int length) {
    for (int i = 0; i < y; i++) {
        printf("\n");
    }
    
    for (int i = 0; i < x; i++) {
        printf(" ");
    }
    
    if (supports_unicode) {
        for (int i = 0; i < length; i++) {
            printf("─");
        }
    } else {
        for (int i = 0; i < length; i++) {
            printf("-");
        }
    }
}

void renderer_draw_vline(int x, int y, int length) {
    for (int i = 0; i < y; i++) {
        printf("\n");
    }
    
    for (int i = 0; i < x; i++) {
        printf(" ");
    }
    
    if (supports_unicode) {
        for (int i = 0; i < length; i++) {
            printf("│\n");
            for (int j = 0; j < x; j++) {
                printf(" ");
            }
        }
    } else {
        for (int i = 0; i < length; i++) {
            printf("|\n");
            for (int j = 0; j < x; j++) {
                printf(" ");
            }
        }
    }
}

void renderer_draw_progress_bar(int x, int y, int width, int percent, Color color) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    int filled = (width * percent) / 100;
    
    for (int i = 0; i < y; i++) {
        printf("\n");
    }
    
    for (int i = 0; i < x; i++) {
        printf(" ");
    }
    
    renderer_set_color(color);
    printf("[");
    
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            printf("█");
        } else {
            printf("░");
        }
    }
    
    printf("] %d%%", percent);
    renderer_reset_color();
}

bool renderer_supports_color(void) {
    return supports_color;
}

bool renderer_supports_unicode(void) {
    return supports_unicode;
}

int renderer_get_width(void) {
    return terminal_width;
}

int renderer_get_height(void) {
    return terminal_height;
}

void renderer_refresh(void) {
    fflush(stdout);
}
