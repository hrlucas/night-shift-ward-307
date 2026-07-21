# Documento de História
## Graveyard Shift

---

### 1. Abordagem Narrativa

Não há uma explicação. A narrativa é
construída inteiramente a partir do que acontece durante um único plantão e de
como o jogo enquadra isso — e é feita para sustentar duas leituras ao mesmo
tempo:

- **Uma literal**: um hospital, uma noite difícil, um(a) enfermeiro(a) fazendo
  o seu melhor (ou não) sob pressão.
- **Uma psicológica**: tudo o que o jogador vive é um episódio dissociativo a partir da sua intuição.

O jogo nunca escolhe uma leitura pelo jogador.

---

### 2. Onde "Ala 307" Aparece

- **Na sobrevivência (07:00)**: quando o personagem bate o cartão, a tela
  mostra "ALA 307" sozinho, centralizado, por cerca de um segundo e meio,
  depois limpa e o jogo segue sem reconhecer — como se nunca tivesse
  acontecido. É, de propósito, a única cena inexplicada num final de resto
  "limpo".
- **No ataque cardíaco**: o personagem é descrito como tendo sido levado à Ala
  307 depois do colapso.
- **No final de colapso psiquiátrico**: a Ala 307 é a própria revelação — o
  jogador desperta numa ala psiquiátrica real e é perguntado se sabe onde está.
  Se aceita, o jogo termina naquela internação. Se nega, o plantão reinicia, e
  `STORY_FLAG_DENIED_ONCE` (ver `narrative/story.h`) causa uma linha extra e
  discreta na próxima tentativa — um sinal, só para quem volta, de que isso já
  aconteceu antes.

---

### 3. Tom e Escalada

O plantão é escrito para parecer comum no começo e acumular estranheza aos
poucos, em vez de escalar por sustos encenados:

- As cenas iniciais são mundanas e fáceis de explicar (um monitor pisca, um
  chamado acaba não sendo nada).
- A primeira cena de fato perturbadora — um chamado rastreado até a própria
  sala do jogador — é colocada de propósito cedo o bastante para ler como uma
  anomalia, ainda não um padrão.
- Os 20 chamados de paciente carregam a maior parte da duração do plantão e do
  seu peso emocional: cada um é uma situação médica contida e concreta, e o
  terror vem do acúmulo deles e do que negligenciar um custa depois, não de
  nenhum momento isolado ser sobrenatural.

---

### 4. Consequência como Narrativa

O dispositivo narrativo central do jogo é o atraso: escolher não agir, ou agir
de forma inadequada, não produz uma falha visível imediata. Produz uma dívida
que vence mais tarde no plantão, na forma de uma mensagem simples — um paciente
que piorou, ou que não resistiu — que o jogador não tem como desfazer. A ideia
é fazer o custo da evasão ser sentido, não apenas calculado: quando a
consequência cai, o momento de ter escolhido diferente já passou.

---

### 5. Diretrizes de Escrita para Novo Conteúdo

Se novos cenários de paciente ou cenas roteirizadas forem adicionados:

- Nunca explique a história do hospital nem confirme qual leitura (literal ou
  psicológica) é a correta.
- Mantenha os anúncios de chamado e as condições dos pacientes concretos e
  específicos — um detalhe nomeado (um número de quarto, um sintoma concreto)
  lê como real; a vagueza lê como suspeita, e esse contraste deve continuar
  significativo, não acidental.
- Qualquer novo final ou cena importante deve decidir, de propósito, se e como
  toca a "Ala 307" — o silêncio também é uma escolha válida, mas deve ser uma
  escolha.
