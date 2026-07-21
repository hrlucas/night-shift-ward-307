#include "systems/patients.h"
#include <stddef.h>

/* Vinte chamados de paciente escritos à mão, formando uma linha cronológica ao
 * longo do turno, com gravidade crescente. Cada linha é dado puro: seus próprios
 * StringIDs de i18n e deltas de aflição. A estrutura uniforme da cena (chamado
 * -> vai? -> condição -> escolher um de três tratamentos -> talvez uma
 * complicação -> talvez uma consequência tardia de negligência) é interpretada
 * por run_patient_scenario em gameplay.c, então os vinte compartilham um único
 * caminho de código.
 *
 * Por opção: `adequate` = a escolha realmente cuida do paciente. Uma escolha
 * inadequada (ou ignorar o chamado por completo) agenda a consequência tardia de
 * negligência do cenário. `administer_index` marca a intervenção clínica
 * arriscada que pode complicar (1-em-`complication_chance_den`); agir é sempre
 * contado como adequado — só a inação/evasão mata alguém. */

static const PatientScenario patient_scenarios[] = {
    /* 01 - Paciente idoso com demência saindo do quarto */
    {
        1, 12, 112, 1,
        STR_SC01_CALL, STR_SC01_COND,
        {
            { STR_SC01_O1, STR_SC01_R1, -3, true },
            { STR_SC01_O2, STR_SC01_R2,  8, false },
            { STR_SC01_O3, STR_SC01_R3, -1, true },
        },
        0, 8, STR_SC01_COMP,
        STR_SC01_NEG, 60, 12,
    },
    /* 02 - Criança com febre muito alta */
    {
        2, 40, 118, 2,
        STR_SC02_CALL, STR_SC02_COND,
        {
            { STR_SC02_O1, STR_SC02_R1, -4, true },
            { STR_SC02_O2, STR_SC02_R2, 10, false },
            { STR_SC02_O3, STR_SC02_R3,  2, true },
        },
        0, 7, STR_SC02_COMP,
        STR_SC02_NEG, 55, 14,
    },
    /* 03 - Ataque de pânico grave (acolher é o certo; sedar pode sair pela culatra) */
    {
        3, 62, 121, 2,
        STR_SC03_CALL, STR_SC03_COND,
        {
            { STR_SC03_O1, STR_SC03_R1,  4, true },
            { STR_SC03_O2, STR_SC03_R2, -4, true },
            { STR_SC03_O3, STR_SC03_R3, 11, false },
        },
        0, 4, STR_SC03_COMP,
        STR_SC03_NEG, 50, 14,
    },
    /* 04 - Emergência diabética; insulina, o horário importa */
    {
        4, 85, 204, 3,
        STR_SC04_CALL, STR_SC04_COND,
        {
            { STR_SC04_O1, STR_SC04_R1, -5, true },
            { STR_SC04_O2, STR_SC04_R2, 12, false },
            { STR_SC04_O3, STR_SC04_R3,  3, true },
        },
        0, 4, STR_SC04_COMP,
        STR_SC04_NEG, 50, 16,
    },
    /* 05 - Queda no corredor, possível fratura */
    {
        5, 105, 0, 2,
        STR_SC05_CALL, STR_SC05_COND,
        {
            { STR_SC05_O1, STR_SC05_R1, -3, true },
            { STR_SC05_O2, STR_SC05_R2,  9, false },
            { STR_SC05_O3, STR_SC05_R3, -1, true },
        },
        0, 6, STR_SC05_COMP,
        STR_SC05_NEG, 60, 14,
    },
    /* 06 - Crise psiquiátrica, paciente delirando */
    {
        6, 125, 307, 3,
        STR_SC06_CALL, STR_SC06_COND,
        {
            { STR_SC06_O1, STR_SC06_R1, -4, true },
            { STR_SC06_O2, STR_SC06_R2,  8, false },
            { STR_SC06_O3, STR_SC06_R3,  2, true },
        },
        0, 4, STR_SC06_COMP,
        STR_SC06_NEG, 55, 16,
    },
    /* 07 - Reação alérgica grave; medicação de emergência */
    {
        7, 148, 210, 4,
        STR_SC07_CALL, STR_SC07_COND,
        {
            { STR_SC07_O1, STR_SC07_R1, -5, true },
            { STR_SC07_O2, STR_SC07_R2, 14, false },
            { STR_SC07_O3, STR_SC07_R3,  6, false },
        },
        0, 3, STR_SC07_COMP,
        STR_SC07_NEG, 45, 18,
    },
    /* 08 - Convulsão / crise epiléptica */
    {
        8, 170, 118, 3,
        STR_SC08_CALL, STR_SC08_COND,
        {
            { STR_SC08_O1, STR_SC08_R1, -4, true },
            { STR_SC08_O2, STR_SC08_R2, -2, true },
            { STR_SC08_O3, STR_SC08_R3, 11, false },
        },
        0, 4, STR_SC08_COMP,
        STR_SC08_NEG, 50, 16,
    },
    /* 09 - Paciente agressivo recusando tratamento */
    {
        9, 190, 305, 3,
        STR_SC09_CALL, STR_SC09_COND,
        {
            { STR_SC09_O1, STR_SC09_R1,  5, true },
            { STR_SC09_O2, STR_SC09_R2,  9, false },
            { STR_SC09_O3, STR_SC09_R3, -2, true },
        },
        0, 3, STR_SC09_COMP,
        STR_SC09_NEG, 50, 16,
    },
    /* 10 - Infecção pós-cirúrgica piorando */
    {
        10, 210, 226, 3,
        STR_SC10_CALL, STR_SC10_COND,
        {
            { STR_SC10_O1, STR_SC10_R1, -5, true },
            { STR_SC10_O2, STR_SC10_R2, 12, false },
            { STR_SC10_O3, STR_SC10_R3,  2, true },
        },
        0, 4, STR_SC10_COMP,
        STR_SC10_NEG, 60, 16,
    },
    /* 11 - Overdose / intoxicação por drogas */
    {
        11, 232, 233, 4,
        STR_SC11_CALL, STR_SC11_COND,
        {
            { STR_SC11_O1, STR_SC11_R1, -5, true },
            { STR_SC11_O2, STR_SC11_R2, 15, false },
            { STR_SC11_O3, STR_SC11_R3,  5, false },
        },
        0, 3, STR_SC11_COMP,
        STR_SC11_NEG, 45, 18,
    },
    /* 12 - Paciente catatônico / sem resposta */
    {
        12, 255, 312, 3,
        STR_SC12_CALL, STR_SC12_COND,
        {
            { STR_SC12_O1, STR_SC12_R1, -3, true },
            { STR_SC12_O2, STR_SC12_R2,  7, false },
            { STR_SC12_O3, STR_SC12_R3, -1, true },
        },
        0, 4, STR_SC12_COMP,
        STR_SC12_NEG, 60, 16,
    },
    /* 13 - Hemorragia; precisa de pressão/transfusão */
    {
        13, 278, 209, 4,
        STR_SC13_CALL, STR_SC13_COND,
        {
            { STR_SC13_O1, STR_SC13_R1, -5, true },
            { STR_SC13_O2, STR_SC13_R2, 16, false },
            { STR_SC13_O3, STR_SC13_R3,  7, false },
        },
        0, 3, STR_SC13_COMP,
        STR_SC13_NEG, 45, 20,
    },
    /* 14 - Interação medicamentosa perigosa (remédio errado já dado antes) */
    {
        14, 300, 214, 4,
        STR_SC14_CALL, STR_SC14_COND,
        {
            { STR_SC14_O1, STR_SC14_R1, -5, true },
            { STR_SC14_O2, STR_SC14_R2, 13, false },
            { STR_SC14_O3, STR_SC14_R3,  3, true },
        },
        0, 3, STR_SC14_COMP,
        STR_SC14_NEG, 50, 18,
    },
    /* 15 - Tentativa de autolesão */
    {
        15, 322, 307, 4,
        STR_SC15_CALL, STR_SC15_COND,
        {
            { STR_SC15_O1, STR_SC15_R1, -4, true },
            { STR_SC15_O2, STR_SC15_R2, 15, false },
            { STR_SC15_O3, STR_SC15_R3,  1, true },
        },
        0, 4, STR_SC15_COMP,
        STR_SC15_NEG, 45, 20,
    },
    /* 16 - Suspeita de AVC / paralisia súbita */
    {
        16, 345, 201, 5,
        STR_SC16_CALL, STR_SC16_COND,
        {
            { STR_SC16_O1, STR_SC16_R1, -6, true },
            { STR_SC16_O2, STR_SC16_R2, 18, false },
            { STR_SC16_O3, STR_SC16_R3,  4, true },
        },
        0, 3, STR_SC16_COMP,
        STR_SC16_NEG, 45, 22,
    },
    /* 17 - Parada respiratória */
    {
        17, 368, 210, 5,
        STR_SC17_CALL, STR_SC17_COND,
        {
            { STR_SC17_O1, STR_SC17_R1, -6, true },
            { STR_SC17_O2, STR_SC17_R2, 20, false },
            { STR_SC17_O3, STR_SC17_R3,  8, false },
        },
        0, 3, STR_SC17_COMP,
        STR_SC17_NEG, 45, 22,
    },
    /* 18 - Paciente em coma, sinais vitais oscilando */
    {
        18, 388, 205, 5,
        STR_SC18_CALL, STR_SC18_COND,
        {
            { STR_SC18_O1, STR_SC18_R1, -5, true },
            { STR_SC18_O2, STR_SC18_R2, 17, false },
            { STR_SC18_O3, STR_SC18_R3,  3, true },
        },
        0, 3, STR_SC18_COMP,
        STR_SC18_NEG, 45, 22,
    },
    /* 19 - Complicação pós-parto */
    {
        19, 402, 240, 5,
        STR_SC19_CALL, STR_SC19_COND,
        {
            { STR_SC19_O1, STR_SC19_R1, -6, true },
            { STR_SC19_O2, STR_SC19_R2, 19, false },
            { STR_SC19_O3, STR_SC19_R3,  5, true },
        },
        0, 3, STR_SC19_COMP,
        STR_SC19_NEG, 45, 22,
    },
    /* 20 - Paciente terminal pedindo companhia (não há solução clínica; a
     * escolha é emocional). Sem intervenção arriscada, logo sem rolagem de
     * complicação. */
    {
        20, 412, 307, 5,
        STR_SC20_CALL, STR_SC20_COND,
        {
            { STR_SC20_O1, STR_SC20_R1, -6, true },
            { STR_SC20_O2, STR_SC20_R2, -2, true },
            { STR_SC20_O3, STR_SC20_R3, 16, false },
        },
        0, 0, STR_SC20_COMP,
        STR_SC20_NEG, 45, 22,
    },
};

#define PATIENT_SCENARIO_COUNT (int)(sizeof(patient_scenarios) / sizeof(patient_scenarios[0]))

/* Quantos cenários existem no total. */
int patient_scenario_count(void) {
    return PATIENT_SCENARIO_COUNT;
}

/* Acesso seguro a um cenário por índice (NULL se fora da faixa). */
const PatientScenario* patient_scenario_get(int index) {
    if (index < 0 || index >= PATIENT_SCENARIO_COUNT) {
        return NULL;
    }
    return &patient_scenarios[index];
}

/* Esvazia a fila de consequências (todos os slots inativos). */
void pending_consequences_init(PendingConsequenceList* list) {
    if (list == NULL) return;

    for (int i = 0; i < MAX_PENDING_TREATMENTS; i++) {
        list->items[i].active = false;
    }
    list->count = 0;
}

/* Agenda uma consequência no primeiro slot livre. Retorna false se a fila
 * estiver cheia (MAX_PENDING_TREATMENTS). */
bool pending_consequences_add(PendingConsequenceList* list, int room_number,
                              StringID consequence_text, int delta,
                              int deadline_total_minutes) {
    if (list == NULL) return false;

    for (int i = 0; i < MAX_PENDING_TREATMENTS; i++) {
        if (!list->items[i].active) {
            list->items[i].active = true;
            list->items[i].room_number = room_number;
            list->items[i].consequence_text = consequence_text;
            list->items[i].delta = delta;
            list->items[i].deadline_total_minutes = deadline_total_minutes;
            if (i >= list->count) {
                list->count = i + 1;
            }
            return true;
        }
    }
    return false;
}

/* Remove e devolve a primeira consequência já vencida (deadline <= agora). */
bool pending_consequences_pop_due(PendingConsequenceList* list, int now_minutes,
                                  PendingConsequence* out) {
    if (list == NULL || out == NULL) return false;

    for (int i = 0; i < list->count; i++) {
        if (list->items[i].active && list->items[i].deadline_total_minutes <= now_minutes) {
            *out = list->items[i];
            list->items[i].active = false;
            return true;
        }
    }
    return false;
}
