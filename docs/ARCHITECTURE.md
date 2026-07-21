# Documento de Arquitetura
## Graveyard Shift

Este documento descreve a base de código como ela realmente é hoje. Todo
arquivo mencionado abaixo é vivo e usado pelo jogo em execução, então este
documento pode ser lido como um mapa completo do projeto.

---

## 1. Princípios de Design

- **Um módulo, uma responsabilidade.** Cada arquivo `.c` cuida de uma única
  preocupação (o loop de jogo, o relógio, o banco de textos, o setup do
  terminal, e assim por diante).
- **Uma única fonte de verdade para o estado.** Tudo sobre uma sessão de jogo
  vive em uma struct, `GameData` (`data/game_state.h`). Nada é duplicado entre
  subsistemas.
- **Sem globais escondidos, exceto a máquina de estados e a config de render.**
  `GameData` é alocado no heap uma vez por `core/game.c` e passado por ponteiro
  em todo o resto.
- **Salvamento simples em arquivo texto.** Uma sessão é um plantão contínuo,
  mas o jogador pode salvar no meio (tecla `0`) e retomar depois. O formato é
  texto puro, linha a linha (`save/save.c`) — sem binário, sem versionamento
  complexo, fácil de explicar.

---

## 2. Mapa de Módulos

```
src/
├── main.c                  Conecta a máquina de estados (boot -> título ->
│                            menu principal -> jogar/carregar -> encerramento)
│                            e é dono das telas de menu.
│
├── core/
│   ├── game.c               Aloca/libera a única instância de GameData, é dono
│   │                         do loop de topo game_run(), detecta capacidades
│   │                         do terminal.
│   ├── state.c               Máquina de estados genérica: registra uma tupla
│   │                         (on_enter/on_exit/update/render/handle_input) por
│   │                         GameState e despacha para o estado atual.
│   ├── i18n.c                Um enum StringID e um único array de textos em
│   │                         português (strings[]). Todo texto visível ao
│   │                         jogador passa por i18n_get(id) — ver seção 4.
│   └── utils.c               Pequenos helpers compartilhados: clamp_int,
│                             random_range, sleep_ms, get_time_ms (relógio
│                             monotônico, base do ritmo em tempo real da seção
│                             3), safe_strcpy.
│
├── data/
│   ├── game_state.c          GameData: a única struct que descreve uma sessão
│   │                         inteira (jogador, perfil do personagem, relógio,
│   │                         flags de história, consequências pendentes, sala,
│   │                         contadores de streak). game_data_init/_reset são
│   │                         donos de tudo; nada é alocado no heap, então não
│   │                         há o que limpar no encerramento.
│   ├── player.c              Player: nome + sanidade (a aflição é só
│   │                         `100 - sanidade%`).
│   ├── character.c           CharacterProfile: nome, data de nascimento
│   │                         (entrada só de dígitos), idade calculada, sexo.
│   └── time.c                GameTime: horas/minutos/total_minutes desde 00:00,
│                             mais time_add_minutes e time_format.
│
├── narrative/
│   └── story.c               StoryFlag: um bitmask de "esta cena roteirizada
│                             one-shot já aconteceu?" (monitor piscando, chamado
│                             da recepção, a ligação de si mesmo, a batida, o
│                             chamado do necrotério, e se o jogador já negou a
│                             revelação psiquiátrica uma vez).
│
├── systems/
│   ├── gameplay.c            O maior arquivo, e o coração do jogo: criação de
│   │                         personagem, o loop do turno em tempo real, todo
│   │                         evento roteirizado, o executor de cena
│   │                         compartilhado dos 20 cenários de paciente, e os
│   │                         três finais. Ver seções 3 e 5.
│   └── patients.c            Dados puros: os 20 cenários de paciente (cada um
│                             uma linha `PatientScenario` de StringIDs e deltas
│                             de aflição) mais a fila de consequências tardias
│                             que faz a negligência voltar mais tarde.
│
├── save/
│   └── save.c                Salvar/carregar em arquivo texto (save.txt): uma
│                             variável por linha, escrita e lida na mesma ordem.
│                             Ver seção 6.
│
└── ui/
    ├── renderer.c            Setup do terminal/console: força saída UTF-8 e
    │                         habilita escapes ANSI no Windows, detecta a
    │                         largura do terminal para centralizar, limpa a
    │                         tela.
    ├── input.c               Leitura de teclas multiplataforma: bloqueante
    │                         (input_get_key), não bloqueante (input_has_input),
    │                         e entrada de texto livre com eco manual
    │                         (input_get_line, necessário porque o modo raw
    │                         desliga o eco do terminal).
    └── box.c                 Helpers de caixa centralizada usados por toda tela
                              (menus, HUD, cenas de paciente, finais). A conta
                              de largura é ciente de UTF-8 (conta caracteres
                              visíveis, não bytes), para o texto acentuado em PT
                              não desalinhar.
```

`include/` espelha isto exatamente: um header por arquivo fonte acima.

---

## 3. O Relógio em Tempo Real

Diferente de um jogo CLI típico por turnos, o relógio do plantão avança
sozinho, sem esperar por input:

- **Ritmo único: 1 minuto de jogo por segundo real, sempre.** Regra simples,
  sem acelerações.
- **O relógio para durante uma decisão.** Quando um chamado/escolha aparece, o
  prompt vira uma leitura bloqueante (`prompt_choice` em `gameplay.c`), sem
  nenhum tick de fundo. Como o relógio realmente congela (em vez de "andar mais
  devagar"), não há risco de o turno passar de 07:00 escondido atrás de uma
  decisão não respondida.

Isso é implementado como um pequeno loop de tick em tempo real no
`gameplay_loop()` (`systems/gameplay.c`): dorme brevemente, mede o tempo real
decorrido via `get_time_ms()`, e o converte em minutos de jogo (1 por segundo).
O texto narrativo (`narrative_beat`) fica na tela por um tempo proporcional ao
seu tamanho, em vez de esperar por tecla, e ainda pode ser pulado por qualquer
tecla.

**Detalhe importante:** depois de qualquer cena bloqueante (uma decisão, um
evento, uma frase ambiente, o menu de pausa), o loop reancora sua base de
medição (`last_tick`/`minute_acc_ms`). Sem isso, o tempo real gasto com o
relógio parado seria convertido de uma vez em minutos de jogo no tique seguinte,
e o relógio "pularia" assim que o jogador escolhesse uma opção.

---

## 4. Banco de Textos (i18n)

Todo texto visível ao jogador é um valor do enum `StringID` em `core/i18n.h`,
resolvido em tempo de execução por `i18n_get(id)`. O jogo tem um idioma único
(português), então não há lógica de seleção de idioma: `i18n_get` apenas
devolve o texto correspondente no array `strings[]` de `core/i18n.c`. Manter
todo o texto num único lugar separa a lógica do conteúdo. Um `StringID` sem
texto preenchido renderiza vazio (`""`) em vez de falhar na compilação, então
texto faltando é um bug de conteúdo a pegar testando, não um erro de build.

---

## 5. Loop de Jogo, Concretamente

0. **Guia de jogo** (`run_guide`): tela curta com o ritmo do relógio e as teclas
   (escolhas, `0` para salvar, `Esc` para pausar), antes de tudo. Só num jogo
   novo — retomar um save cai direto no loop.
1. **Criação de personagem** (`run_character_creation`): nome, data de
   nascimento (só dígitos, ex. `15061990`, sem exigir separadores), sexo, idade
   calculada a partir do relógio real do sistema.
2. **Intro + cenas de abertura**: uma cena curta estabelecendo o cenário e o
   peso emocional, sem pista da revelação.
3. **O loop do turno** (`gameplay_loop`): a cada tick, em ordem —
   - avança o relógio (1 min/seg) e rola o gotejar de aflição de fundo;
   - checa as três condições de fim (ver abaixo);
   - dispara o próximo evento agendado, se chegou a hora
     (`check_and_fire_events`) — inclui as cinco cenas atmosféricas iniciais e
     os 20 cenários de paciente (`run_patient_scenario`, movido por dados em
     `patients.c`);
   - checa qualquer consequência tardia vencendo (`check_pending_consequences`
     — o preço de ter negligenciado um paciente antes);
   - talvez dispare uma frase ambiente de clima (algumas elevam a aflição);
   - redesenha o HUD se o minuto exibido mudou;
   - trata teclas fora de um chamado: `0` salva a partida na hora; `Esc` abre o
     menu de pausa (continuar / salvar e continuar / salvar e sair / sair sem
     salvar). Se o Esc foi apertado durante uma cena narrativa — que consome a
     tecla para pular o texto — a flag `pause_requested` faz o loop abrir o menu
     na iteração seguinte, para o Esc nunca se perder;
   - reancora a base de medição do relógio se alguma cena rodou (ver seção 3).
4. **Finais**, checados nesta ordem a cada tick:
   - **Aflição chega a 100%** → ataque cardíaco (imediato, sem escolha).
   - **`ignore_streak` chega ao limiar** → a sequência de colapso psiquiátrico:
     o jogador desperta numa ala real e deve aceitar (final) ou negar (o turno
     reinicia com o mesmo personagem, aflição cheia, e uma flag —
     `STORY_FLAG_DENIED_ONCE` — que mostra uma linha extra da próxima vez).
   - **Relógio chega às 07:00** → sobreviver ao plantão.

Ao alcançar qualquer final, o `save.txt` é apagado (aquela partida acabou).
Abandonar com Esc não é um final, então o save é preservado.

Todo chamado — ambiente ou cenário de paciente — abre com uma de algumas linhas
sorteadas de "como o chamado chega até você" (um telefone na recepção, o
interfone, uma voz no corredor, …) antes do conteúdo específico, para o
enquadramento sensorial não se repetir igual toda vez.

---

## 6. Formato do Save

`save/save.c` grava `save.txt` com um valor por linha, na ordem em que aparecem
em `save_game()`; `load_game()` lê exatamente na mesma ordem. Persistimos:
perfil do personagem (nome, data de nascimento, idade, sexo), jogador (nome,
sanidade), o total de minutos do relógio (hora/minuto são reconstruídos no
load), o bitmask de flags de história, os contadores do ritmo do turno e do
streak de evasão, a sala atual, e a fila de consequências pendentes.

Detalhe importante: os nomes podem conter espaços, então cada nome é gravado na
sua própria linha e lido com `fgets` (não `fscanf("%s")`, que pararia no
primeiro espaço).

---

## 7. Sistema de Build

O caminho principal é o **Makefile** (`make`). O `CMakeLists.txt` existe como
alternativa opcional. Ambos descobrem os arquivos fonte automaticamente:

- **Makefile**: `SOURCES = $(wildcard src/*.c src/*/*.c)`.
- **CMakeLists.txt**: `file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c")`.

Observação sobre o Makefile: ele cria um diretório de build por subpasta de
`src/`. Ao adicionar uma subpasta nova em `src/` (como foi feito com `src/save`),
inclua o diretório correspondente na linha `mkdir -p` do alvo `$(OBJDIR)`.

Ambos compilam sob `-std=c11 -Wall -Wextra -Werror`; um build limpo com zero
avisos é requisito obrigatório para qualquer mudança neste projeto.
