#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

/**
 * @file input.h
 * @brief Sistema de tratamento de input.
 */

/**
 * @brief Teclas de entrada reconhecidas.
 */
typedef enum {
    INPUT_NONE,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_ENTER,
    INPUT_ESCAPE,
    INPUT_1,
    INPUT_2,
    INPUT_3,
    INPUT_4,
    INPUT_5,
    INPUT_6,
    INPUT_7,
    INPUT_8,
    INPUT_9,
    INPUT_0,
    INPUT_Q,
    INPUT_W,
    INPUT_E,
    INPUT_R,
    INPUT_S,
    INPUT_UNKNOWN
} InputKey;

/**
 * @brief Inicializa o sistema de input.
 */
void input_init(void);

/**
 * @brief Finaliza o sistema de input.
 */
void input_shutdown(void);

/**
 * @brief Lê a próxima tecla (bloqueante). Um stdin fechado (EOF) vira
 *        INPUT_ESCAPE, para nenhum laço de leitura girar para sempre.
 */
InputKey input_get_key(void);

/**
 * @brief Verifica se há input disponível (não bloqueante).
 */
bool input_has_input(void);

/**
 * @brief Lê o input como caractere.
 */
char input_get_char(void);

/**
 * @brief Lê o input como inteiro.
 */
int input_get_int(void);

/**
 * @brief Limpa o buffer de input.
 */
void input_clear(void);

/**
 * @brief Espera por qualquer tecla.
 */
void input_wait_for_key(void);

/**
 * @brief Lê uma linha de texto do jogador, com eco visível e suporte a
 *        backspace (necessário porque o modo raw/não-canônico desliga o eco do
 *        terminal).
 * @param buffer Buffer de destino.
 * @param max_len Número máximo de caracteres (incluindo o terminador nulo).
 */
void input_get_line(char* buffer, int max_len);

#endif /* INPUT_H */
