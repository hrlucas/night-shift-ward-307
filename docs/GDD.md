# Documento de Design de Jogo
## Graveyard Shift

---

### Metadados

- **Gênero**: Terror psicológico, baseado em terminal
- **Plataforma**: CLI (Linux, Windows via MinGW-w64)
- **Linguagem**: C
- **Duração**: Um plantão, mais ou menos 10–20 minutos de tempo real
- **Modelo de sessão**: um plantão contínuo, com salvamento opcional em arquivo texto (salvar no meio e retomar depois)
- **Idioma**: Português 
---

### 1. Premissa

Você é enfermeiro(a) no primeiro plantão da madrugada, sozinho(a), das 00:00
às 07:00. Ninguém te diz o que é certo. Cada chamado que chega é uma decisão
sua, e cada decisão tem um custo real — para a sua própria compostura, e para
quem você esteve, ou não esteve, presente.

O jogo deliberadamente segura uma grande revelação explicada de lore. O que
sobra para o jogador é um único sinal ambíguo — "ALA 307" — que aparece nas
bordas de todos os finais, nunca explicado, nunca confirmado como real ou
imaginado.

---

### 2. Pilares Centrais

- **Um relógio que não espera por você.** O tempo é real, não por turnos —
  ver seção 3.
- **Toda escolha tem uma forma, não só um valor.** Uma escolha "segura"
  (atender, abrir a porta) não é de graça para sempre, e uma "evasiva" não é
  barata na segunda vez.
- **As consequências chegam atrasadas.** Negligenciar um paciente não pune na
  hora — volta, no próprio ritmo, mais tarde no plantão.
- **Sem sustos baratos, sem monstros.** O terror é procedural e atmosférico:
  um flash inexplicado de "Ala 307", uma ligação que veio da sua própria sala,
  uma ala psiquiátrica que pode ou não ser mais real do que o hospital onde
  você achava que estava.

---

### 3. O Relógio

- Corre de **00:00 a 07:00** automaticamente, em tempo real.
- **Ritmo único: 1 minuto de jogo por segundo real, sempre.** Regra simples e
  fácil de explicar.
- **O relógio para durante uma decisão.** Quando um chamado ou escolha aparece,
  o tempo congela por completo (a leitura da escolha é bloqueante) e só volta a
  andar depois que o jogador responde. Assim o turno nunca passa de 07:00
  escondido atrás de uma decisão não respondida.
- Não há ação manual de "aguardar" ou "patrulhar". O jogador está sempre lendo
  algo que o plantão está dizendo, ou decidindo sobre isso.

---

### 4. Chamados e Pacientes

**Cinco cenas atmosféricas iniciais** (um monitor piscando, um chamado para uma
recepção vazia, um telefone que acabou tocando da sua própria sala, uma batida
que ninguém atende, um pedido para conferir o necrotério) estabelecem o tom
antes de os chamados de paciente assumirem como mecânica principal.

**Vinte cenários de paciente escritos à mão** formam a espinha do plantão, cada
um seguindo a mesma forma:

1. Um chamado chega, anunciado por uma de várias deixas ambientes rotativas
   (um telefone, um interfone, uma voz, uma luz de chamada, passos), para o
   enquadramento variar.
2. O jogador é perguntado se vai — ignorar é sempre possível, e sempre tem um
   custo.
3. Ao chegar, a condição do paciente é descrita.
4. Uma decisão real de tratamento com três opções (nunca um sim/não binário).
5. A intervenção escolhida pode **complicar** (chance ajustável): se complica,
   segue uma segunda decisão — chamar ajuda (custo pequeno) ou sair na surdina
   (custo maior, e conta como escolha evasiva).
6. Uma escolha inadequada — ou ignorar o chamado por completo — agenda uma
   **consequência tardia**: o quadro do paciente piora ou o paciente morre,
   entregue como mensagem mais tarde no plantão, batendo na aflição mais forte
   do que qualquer desfecho de ter realmente ajudado.

Os arquétipos de paciente cobrem uma ampla faixa de urgência e tom: um idoso
confuso, uma criança com febre, um ataque de pânico, uma emergência diabética,
uma crise psiquiátrica, uma queda no corredor, e mais — escalando em gravidade
conforme o plantão avança.

**Frases ambientes de clima** aparecem aleatoriamente entre os eventos
principais. A maioria é puro clima; algumas ("Sua aflição aumenta...") também
elevam a aflição, para manter o jogador em alerta constante.

---

### 5. Aflição e o Streak de Evasão

- **Aflição** (0–100%) é o medidor de pressão do plantão. É derivada
  diretamente da sanidade do personagem (`100 - sanidade%`). Bons desfechos
  aliviam um pouco; ruins — especialmente negligência — elevam mais.
- **`ignore_streak`** conta especificamente as escolhas evasivas consecutivas.
  Zera no instante em que o jogador se engaja, e decai sozinho após um trecho
  sem nova evasão, então um começo ruim não é derrota automática.

---

### 6. Finais

Checados nesta prioridade a cada tique do relógio:

1. **Ataque cardíaco** — a aflição chega a 100%. Abrupto, sem escolha final,
   enquadrado como ser levado à Ala 307 depois.
2. **Colapso psiquiátrico** — o streak de evasão cruza seu limiar. O jogador
   desperta numa ala psiquiátrica real e é perguntado se sabe onde está:
   - **Aceitar** → o plantão termina ali, naquela internação.
   - **Negar** → o plantão reinicia silenciosamente em 00:00 com o mesmo
     personagem, aflição recuperada, mas com uma linha extra sutil da próxima
     vez, sugerindo que isso já aconteceu antes.
3. **Sobreviver** — o relógio chega às 07:00. Na saída, um único flash
   inexplicado de "Ala 307" aparece na tela por um instante antes de o jogo
   seguir como se nada tivesse acontecido — o final "bom" é, de propósito, não
   limpo.

Ao alcançar qualquer um desses finais, o `save.txt` é apagado: aquela partida
terminou e não pode ser recarregada.

---

### 7. Salvamento e Pausa

Durante o plantão, a tecla `0` salva a partida em um arquivo texto único
(`save.txt`) e o jogo continua na hora — bem diferente de "sair e perder o
progresso".

A tecla `Esc` abre o **menu de pausa**, com o relógio parado, oferecendo:
continuar o plantão, salvar e continuar, salvar e sair, ou sair sem salvar.

No menu principal, "Carregar Jogo" lê o arquivo e retoma exatamente de onde
parou (personagem, minutos passados, aflição, eventos já ocorridos, contador de
escolhas evasivas). Essa opção **só aparece quando existe um save no disco** —
sem save, o menu tem apenas "Jogar" e "Sair". Ver `docs/ARCHITECTURE.md` para o
formato do arquivo.

### 7.1 Guia de jogo

Ao começar um plantão novo, antes da criação de personagem e da história, o jogo
mostra uma tela curta de **como jogar**: o ritmo do relógio, as teclas numéricas
para escolher opções, o fato de o relógio parar durante uma decisão, `0` para
salvar e `Esc` para pausar.

---

### 8. Idioma

O jogo é inteiramente em **português**, com um idioma único. Todo texto visível
ao jogador passa pelo módulo `i18n.c`, que funciona como um banco central de
textos (separação entre lógica e conteúdo), sem lógica de seleção de idioma.
A convenção do projeto é: identificadores e lógica em inglês, comentários e
documentação em português.

---

### 9. O Que Deliberadamente Não Está Nesta Versão

Para manter a base de código pequena o suficiente para explicar de ponta a
ponta, várias ideias de rodadas de design anteriores foram cortadas em vez de
meio-construídas: um sistema de inventário, uma mecânica de lanterna/bateria,
um mapa de parede para descobrir, e navegação livre entre salas. Se qualquer
uma voltar, deve voltar como uma adição deliberada e com escopo — não como
esqueleto sobrando.
