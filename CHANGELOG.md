# Changelog

Todas as mudanças relevantes em Graveyard Shift são documentadas neste
arquivo.

O formato é baseado no [Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/),
e o projeto segue o [Versionamento Semântico](https://semver.org/lang/pt-BR/).

## [Não lançada]

### Renomeação, menu de pausa e correções de relógio

#### Corrigido
- **Esc não abria o menu de saída.** As cenas narrativas consomem qualquer tecla
  para pular o texto, e acabavam "engolindo" o Esc. Agora `narrative_wait()`
  marca a flag `pause_requested` quando a tecla consumida é Esc, e o loop
  principal abre o menu de pausa na iteração seguinte.
- **O relógio pulava depois de uma escolha.** O tempo ficava visualmente parado
  durante a decisão (correto), mas o loop convertia todo o tempo real gasto no
  prompt em minutos de jogo no tique seguinte. O loop agora reancora a base de
  medição (`last_tick`/`minute_acc_ms`) depois de qualquer cena bloqueante —
  decisão, evento, consequência, frase ambiente, menu de pausa ou salvamento.

#### Adicionado
- **Menu de pausa** (Esc durante o plantão, com o relógio parado): continuar,
  salvar e continuar, salvar e sair, ou sair sem salvar. Substitui o antigo
  "Sair agora? Sim/Não".
- **Guia de jogo** mostrado ao começar um plantão novo, antes da história:
  explica o ritmo do relógio, as teclas numéricas, o `0` para salvar e o Esc
  para pausar.

#### Alterado
- **Projeto renomeado para Graveyard Shift** (`graveyard-shift-game`): tela de
  título, documentação, `CMakeLists.txt` e o executável, que passou a se chamar
  `graveyard_shift`. O subtítulo "Plantão da Madrugada" foi removido.
- **"Carregar Jogo" só aparece no menu quando existe um save**; sem save o menu
  tem apenas Jogar e Sair (e "Sair" passa a ser a tecla 2).
- `save.txt` adicionado ao `.gitignore`.

### Rodada de entrega acadêmica

Preparação do projeto para uma entrega acadêmica (vídeo explicativo curto e
defesa oral), mantendo a arquitetura modular multi-arquivo.

#### Adicionado
- **Salvamento em arquivo texto** (`src/save/save.c`): um único `save.txt`, com
  uma variável por linha, escrito e lido na mesma ordem. Durante o plantão, a
  tecla `0` salva na hora e o jogo continua; no menu principal, "Carregar Jogo"
  retoma exatamente de onde parou (personagem, minutos, aflição, eventos já
  ocorridos, streak de evasão). Ao chegar a um dos finais, o save é apagado;
  abandonar com Esc preserva o save.
- Novas frases ambientes de clima espalhadas pelo turno; algumas ("Sua aflição
  aumenta...") também elevam a aflição, reaproveitando o mecanismo de stingers.

#### Alterado
- **Idioma único: português.** Removida a seleção de idioma por completo (enum
  `Language`, o array `strings_en[]`, `i18n_set_language`/`i18n_get_language`, o
  estado/menu de idioma). O `i18n.c` continua como banco central de textos, agora
  com um único array em português.
- **Ritmo de tempo simplificado para 1:1** (1 minuto de jogo por segundo real,
  sempre) e o **relógio agora para durante uma decisão**: `prompt_choice` virou
  uma leitura bloqueante, sem timeout nem tick de fundo, o que elimina qualquer
  risco de o turno passar de 07:00 atrás de uma decisão não respondida.
- **Comentários do código traduzidos para português** em todos os `.c`/`.h`
  (identificadores e lógica permanecem em inglês).
- **Documentação unificada em português**: um único `README.md` em português
  (o README em inglês e o `README.pt-BR.md` foram removidos); `docs/*.md`
  traduzidos e atualizados; um caminho de build principal por sistema.
- Autoria atualizada para **Lucas Hochmann Rosa** em LICENSE, README e
  comentários (removido o domínio/handle antigo).

#### Removido
- **Tela de créditos** por completo (`STATE_CREDITS`, o item de menu, os
  callbacks e as strings `STR_CREDITS_*`).
- **Pasta `tests/`** (testes em Python) e suas menções na documentação.
- Constantes mortas em `core/utils.h` que referenciavam sistemas já removidos.

### Correções de relógio e ritmo (rodadas anteriores)
- **Crítico**: deixar o jogo ocioso durante uma decisão fazia o relógio do
  plantão correr indefinidamente além das 07:00. À época, `prompt_choice()`
  ganhou um timeout de 10 s; nesta rodada o prompt passou a ser bloqueante com
  o relógio pausado, resolvendo a raiz do problema.
- **Crítico**: o primeiro evento roteirizado (um monitor piscando, sem decisão)
  consumia a mesma janela silenciosa de 15–90 minutos usada para espaçar os
  chamados reais, atrasando o primeiro chamado. Cenas puramente ambientes
  deixaram de mexer nesse cooldown.
- A narração de intro/abertura estava avançando o relógio antes de o plantão
  começar; foi adicionada uma variante de espera que não avança o tempo, usada
  só antes do turno.

### Passe de simplificação (rodadas anteriores)
- Removido todo o esqueleto não usado de iterações de design antigas (grafo de
  salas/mundo, entidades de NPC/item/documento, sistema de diálogo, inventário
  e objetivos, rastreador de escolhas e finais ponderados, widgets antigos de
  HUD/menu, e a mecânica de lanterna/bateria).
- Adicionado o conjunto de frases ambientes de "como o chamado chega até você",
  sorteadas antes do conteúdo específico de cada chamado.
- Reescritos `docs/GDD.md`, `docs/ARCHITECTURE.md` e `docs/STORY.md`, que haviam
  ficado muito desatualizados ao longo dos redesenhos.

### i18n e correções de estabilidade (rodadas anteriores)
- Introdução do sistema de i18n com tabela de textos e centralização de todo o
  texto visível ao jogador.
- **Crítico**: mojibake de caracteres de caixa e acentos no Windows —
  `renderer_init()` não estava sendo chamado; passou a ser chamado em `main()`,
  configurando o console para UTF-8 e habilitando escapes ANSI.
- **Crítico**: detecção de largura do terminal sem checar o retorno da chamada
  podia usar memória não inicializada; agora cai no padrão 80x24 em caso de
  falha.
- **Crítico**: `input_get_key()` mapeia `EOF` para `INPUT_ESCAPE`, para um stdin
  fechado nunca girar um laço a 100% de CPU.

## [1.0.0] - 2026-07-08

### Adicionado
- Primeira versão de Graveyard Shift.
- Arquitetura modular completa, máquina de estados, sistema de tempo, e a base
  do jogo.

> Observação: a 1.0.0 original incluía vários sistemas (inventário, lanterna,
> save binário, múltiplos finais) que foram deliberadamente cortados ou
> retrabalhados nas rodadas seguintes descritas acima.

---

## Histórico de Versões

- **1.0.0** (2026-07-08): Primeira versão.
