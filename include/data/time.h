#ifndef TIME_H
#define TIME_H

#include <stdbool.h>
#include "core/utils.h"

/**
 * @file time.h
 * @brief O relógio do plantão: minutos desde 00:00.
 */

typedef struct {
    int hours;
    int minutes;
    int total_minutes;  /* minutos desde 00:00 */
} GameTime;

/**
 * @brief Inicializa o relógio em 00:00.
 */
void time_init(GameTime* time);

/**
 * @brief Avança o relógio um número de minutos.
 */
void time_add_minutes(GameTime* time, int minutes);

/**
 * @brief Formata como "HH:MM".
 */
void time_format(const GameTime* time, char* buffer, size_t buffer_size);

#endif /* TIME_H */
