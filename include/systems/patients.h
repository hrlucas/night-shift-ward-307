#ifndef PATIENTS_H
#define PATIENTS_H

#include <stdbool.h>
#include "core/utils.h"
#include "core/i18n.h"

/**
 * @file patients.h
 * @brief Cenários de chamado de paciente escritos à mão, mais a fila de
 *        consequências tardias que move os desfechos de negligência.
 *
 *        Cada cenário é uma linha de dados (com seus próprios StringIDs de i18n
 *        e deltas de aflição). Um único executor de cena em gameplay.c
 *        interpreta qualquer cenário, então os 20 casos compartilham um só
 *        caminho de código em vez de 20 funções copiadas. Para adicionar um
 *        cenário: acrescente uma linha a patient_scenarios[] em patients.c e
 *        seus textos em i18n.c — nenhum código novo é necessário.
 *
 *        Só dados puros + funções puras (sem UI, sem alocação). As structs
 *        guardam apenas ints/enums/StringIDs.
 */

/* Uma opção de tratamento oferecida quando o jogador chega ao paciente. */
typedef struct {
    StringID label;    /* texto do botão, ex. "[1] Aplicar insulina" */
    StringID outcome;  /* o que acontece se escolhida (caminho sem complicação) */
    int delta;         /* delta de aflição (negativo = alívio) */
    bool adequate;     /* true se trata o paciente corretamente; se false,
                          agenda-se uma consequência tardia de negligência */
} TreatmentOption;

typedef struct {
    int id;
    int call_minute;              /* minuto de jogo em que o chamado dispara */
    int room_number;
    int severity;                 /* 1..5, guardado para balanceamento/telemetria */
    StringID call_text;           /* urgência em uma linha (passo 1) */
    StringID condition_text;      /* estado do paciente ao chegar (passo 3) */
    TreatmentOption options[3];   /* três escolhas reais de tratamento (passo 4) */
    int administer_index;         /* qual opção é a intervenção arriscada */
    int complication_chance_den;  /* 0 = nunca; senão chance 1-em-N de dar errado */
    StringID complication_text;   /* mostrado quando a intervenção complica */
    StringID neglect_text;        /* consequência tardia (ignorar / inadequado) */
    int neglect_delay_min;        /* quanto tempo até essa consequência cair */
    int delta_neglect;            /* golpe de aflição quando a negligência dispara (grande) */
} PatientScenario;

int patient_scenario_count(void);
const PatientScenario* patient_scenario_get(int index);

/* Uma consequência agendada para cair mais tarde (o paciente que você ignorou
 * ou tratou mal piora depois de neglect_delay_min minutos de jogo). */
typedef struct {
    bool active;
    int room_number;
    StringID consequence_text;
    int delta;                    /* golpe de aflição quando cai */
    int deadline_total_minutes;   /* minuto de jogo absoluto em que vence */
} PendingConsequence;

typedef struct {
    PendingConsequence items[MAX_PENDING_TREATMENTS];
    int count;
} PendingConsequenceList;

void pending_consequences_init(PendingConsequenceList* list);
bool pending_consequences_add(PendingConsequenceList* list, int room_number,
                              StringID consequence_text, int delta,
                              int deadline_total_minutes);

/* Remove (desativa) e devolve uma entrada vencida (deadline <= now_minutes) por
 * *out, se houver. Retorna false e deixa *out intacto quando nada venceu.
 * Chame em loop para drenar toda entrada que venceu no mesmo tick. */
bool pending_consequences_pop_due(PendingConsequenceList* list, int now_minutes,
                                  PendingConsequence* out);

#endif /* PATIENTS_H */
