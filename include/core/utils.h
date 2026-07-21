#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @file utils.h
 * @brief Funções utilitárias e constantes comuns.
 */

/* Tamanho máximo de nomes (jogador e personagem). */
#define MAX_NAME_LENGTH 64

/* Número máximo de consequências tardias em espera na fila. */
#define MAX_PENDING_TREATMENTS 8

/* Limites de sanidade (a aflição é 100 - sanidade%). */
#define MAX_SANITY 100
#define MIN_SANITY 0
#define STARTING_SANITY 100

/**
 * @brief Cópia segura de string com terminação nula garantida.
 */
void safe_strcpy(char* dest, const char* src, size_t dest_size);

/**
 * @brief Verifica se a string é vazia ou nula.
 */
bool is_string_empty(const char* str);

/**
 * @brief Gera um número aleatório no intervalo [min, max].
 */
int random_range(int min, int max);

/**
 * @brief Limita um inteiro ao intervalo [min, max].
 */
int clamp_int(int value, int min, int max);

/**
 * @brief Limpa a tela do terminal.
 */
void clear_screen(void);

/**
 * @brief Dorme por N milissegundos (independente de plataforma).
 */
void sleep_ms(int milliseconds);

/**
 * @brief Tempo monotônico em milissegundos (para o ritmo em tempo real).
 */
long long get_time_ms(void);

#endif /* UTILS_H */
