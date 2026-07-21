#include "data/character.h"
#include <string.h>
#include <time.h>

/* Preenche o perfil com valores padrão seguros. */
void character_init(CharacterProfile* profile) {
    if (profile == NULL) return;

    memset(profile->name, 0, sizeof(profile->name));
    profile->birth_day = 1;
    profile->birth_month = 1;
    profile->birth_year = 2000;
    profile->age = 0;
    profile->sex = PLAYER_SEX_MALE;
}

/* Calcula a idade em anos completos comparando a data de nascimento com a data
 * real de hoje (via localtime). */
int character_calculate_age(int birth_day, int birth_month, int birth_year) {
    time_t now = time(NULL);
    struct tm* local = localtime(&now);

    if (local == NULL) {
        return 0;
    }

    int cur_year = local->tm_year + 1900;
    int cur_month = local->tm_mon + 1;
    int cur_day = local->tm_mday;

    int age = cur_year - birth_year;

    /* Ainda não fez aniversário este ano: desconta um. */
    if (cur_month < birth_month ||
        (cur_month == birth_month && cur_day < birth_day)) {
        age--;
    }

    if (age < 0) {
        age = 0;
    }

    return age;
}
