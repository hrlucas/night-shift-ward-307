#define _DEFAULT_SOURCE
#include "core/utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* Cópia de string que sempre termina em '\0', evitando estouro de buffer. */
void safe_strcpy(char* dest, const char* src, size_t dest_size) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return;
    }
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

/* True se a string é nula ou começa com o terminador (vazia). */
bool is_string_empty(const char* str) {
    return str == NULL || str[0] == '\0';
}

/* Inteiro aleatório em [min, max]. Semeia o gerador uma única vez, na primeira
 * chamada, com o relógio do sistema. */
int random_range(int min, int max) {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
    return min + (rand() % (max - min + 1));
}

/* Limita `value` ao intervalo [min, max]. */
int clamp_int(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/* Limpa a tela (comando do SO). */
void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* Dorme por N milissegundos (API específica de cada plataforma). */
void sleep_ms(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

/* Relógio monotônico em milissegundos, base do ritmo em tempo real. */
long long get_time_ms(void) {
#ifdef _WIN32
    return (long long)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}
