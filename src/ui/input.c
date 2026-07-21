/*
 * Leitura de teclas multiplataforma. No Windows usa conio (_getch/_kbhit); no
 * POSIX coloca o terminal em modo raw (sem canônico, sem eco) para ler tecla a
 * tecla. Como o eco fica desligado, input_get_line() reescreve na tela cada
 * caractere digitado por conta própria.
 */
#include "ui/input.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

/* No POSIX, desliga o modo canônico e o eco do terminal (no Windows o conio já
 * lê tecla a tecla, então não há o que configurar). */
void input_init(void) {
#ifdef _WIN32

#else
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) == 0) {
        term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }
#endif
}

/* Restaura o modo canônico e o eco ao encerrar. */
void input_shutdown(void) {
#ifdef _WIN32
    
#else
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) == 0) {
        term.c_lflag |= (ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }
#endif
}

/* Lê uma tecla (bloqueante) e a mapeia para o enum InputKey. Setas geram um
 * código de escape em duas partes no Windows; teclas não mapeadas viram
 * INPUT_UNKNOWN. */
InputKey input_get_key(void) {
#ifdef _WIN32
    int ch = _getch();

    if (ch == 0 || ch == 224) {
        ch = _getch();
        
        switch (ch) {
            case 72: return INPUT_UP;
            case 80: return INPUT_DOWN;
            case 75: return INPUT_LEFT;
            case 77: return INPUT_RIGHT;
            default: return INPUT_UNKNOWN;
        }
    }
    
    switch (ch) {
        case 13: return INPUT_ENTER;
        case 27: return INPUT_ESCAPE;
        case '1': return INPUT_1;
        case '2': return INPUT_2;
        case '3': return INPUT_3;
        case '4': return INPUT_4;
        case '5': return INPUT_5;
        case '6': return INPUT_6;
        case '7': return INPUT_7;
        case '8': return INPUT_8;
        case '9': return INPUT_9;
        case '0': return INPUT_0;
        case 'q': case 'Q': return INPUT_Q;
        case 'w': case 'W': return INPUT_W;
        case 'e': case 'E': return INPUT_E;
        case 'r': case 'R': return INPUT_R;
        case 's': case 'S': return INPUT_S;
        default: return INPUT_UNKNOWN;
    }
#else
    int c = getchar();

    /* stdin fechado/exaurido (EOF) vira ESCAPE, para nenhum laço de leitura
     * girar para sempre a 100% de CPU. */
    if (c == EOF) return INPUT_ESCAPE;

    char ch = (char)c;
    
    switch (ch) {
        case '\n': case '\r': return INPUT_ENTER;
        case 27: return INPUT_ESCAPE;
        case '1': return INPUT_1;
        case '2': return INPUT_2;
        case '3': return INPUT_3;
        case '4': return INPUT_4;
        case '5': return INPUT_5;
        case '6': return INPUT_6;
        case '7': return INPUT_7;
        case '8': return INPUT_8;
        case '9': return INPUT_9;
        case '0': return INPUT_0;
        case 'q': case 'Q': return INPUT_Q;
        case 'w': case 'W': return INPUT_W;
        case 'e': case 'E': return INPUT_E;
        case 'r': case 'R': return INPUT_R;
        case 's': case 'S': return INPUT_S;
        case 'A': return INPUT_UP;
        case 'B': return INPUT_DOWN;
        case 'D': return INPUT_LEFT;
        case 'C': return INPUT_RIGHT;
        default: return INPUT_UNKNOWN;
    }
#endif
}

bool input_has_input(void) {
#ifdef _WIN32
    return _kbhit();
#else
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    int ch = getchar();
    
    fcntl(STDIN_FILENO, F_SETFL, flags);
    
    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }
    
    return false;
#endif
}

char input_get_char(void) {
#ifdef _WIN32
    return (char)_getch();
#else
    return (char)getchar();
#endif
}

int input_get_int(void) {
    char buffer[32];
    int i = 0;
    
    while (i < 31) {
        char ch = input_get_char();
        
        if (ch == '\n' || ch == '\r') {
            buffer[i] = '\0';
            break;
        }
        
        if (ch >= '0' && ch <= '9') {
            buffer[i++] = ch;
        }
    }
    
    buffer[31] = '\0';
    return atoi(buffer);
}

void input_clear(void) {
#ifdef _WIN32
    while (_kbhit()) {
        _getch();
    }
#else
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    while (getchar() != EOF);
    
    fcntl(STDIN_FILENO, F_SETFL, flags);
#endif
}

void input_wait_for_key(void) {
#ifdef _WIN32
    _getch();
#else
    getchar();
#endif
}

void input_get_line(char* buffer, int max_len) {
    if (buffer == NULL || max_len <= 0) {
        return;
    }

    int len = 0;
    buffer[0] = '\0';

    while (1) {
#ifdef _WIN32
        int ch = _getch();
#else
        int ch = getchar();
#endif

        if (ch == EOF) {
            break;
        }

        if (ch == '\n' || ch == '\r') {
            putchar('\n');
            break;
        }

        if (ch == 127 || ch == 8) { /* backspace / DEL */
            if (len > 0) {
                len--;
                printf("\b \b");   /* apaga o último caractere na tela */
                fflush(stdout);
            }
            continue;
        }

        /* Só aceita ASCII imprimível, para manter simples e seguro. */
        if (ch >= 32 && ch < 127 && len < max_len - 1) {
            buffer[len++] = (char)ch;
            putchar(ch);
            fflush(stdout);
        }
    }

    buffer[len] = '\0';
}
