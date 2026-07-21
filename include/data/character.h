#ifndef CHARACTER_H
#define CHARACTER_H

#include "core/utils.h"

/**
 * @file character.h
 * @brief Perfil do personagem, criado no início de um novo jogo.
 */

typedef enum {
    PLAYER_SEX_MALE,
    PLAYER_SEX_FEMALE
} PlayerSex;

typedef struct {
    char name[MAX_NAME_LENGTH];
    int birth_day;
    int birth_month;
    int birth_year;
    int age;
    PlayerSex sex;
} CharacterProfile;

/**
 * @brief Inicializa um perfil de personagem com valores padrão seguros.
 */
void character_init(CharacterProfile* profile);

/**
 * @brief Calcula a idade em anos completos, da data de nascimento até hoje.
 */
int character_calculate_age(int birth_day, int birth_month, int birth_year);

#endif /* CHARACTER_H */
