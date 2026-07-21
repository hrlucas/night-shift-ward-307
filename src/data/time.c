#include "data/time.h"
#include <stdio.h>

/* Zera o relógio em 00:00 (início do plantão). */
void time_init(GameTime* time) {
    if (time == NULL) return;

    time->hours = 0;
    time->minutes = 0;
    time->total_minutes = 0;
}

/* Avança o relógio: total_minutes é o campo canônico, e hora/minuto são
 * derivados dele a cada avanço. */
void time_add_minutes(GameTime* time, int minutes) {
    if (time == NULL) return;

    time->total_minutes += minutes;
    time->hours = (time->total_minutes / 60) % 24;
    time->minutes = time->total_minutes % 60;
}

/* Formata a hora atual como "HH:MM" no buffer recebido. */
void time_format(const GameTime* time, char* buffer, size_t buffer_size) {
    if (time == NULL || buffer == NULL || buffer_size < 6) return;

    snprintf(buffer, buffer_size, "%02d:%02d", time->hours, time->minutes);
}
