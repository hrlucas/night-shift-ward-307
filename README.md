# 🏥 Graveyard Shift

<p align="center">
  <a href="https://github.com/lucas-hochmann-rosa/graveyard-shift-game">
    <img src="https://img.shields.io/badge/GitHub-graveyard--shift--game-181717?style=for-the-badge&logo=github">
  </a>
  <a href="https://www.linkedin.com/in/lucas-hochmann-rosa">
    <img src="https://img.shields.io/badge/LinkedIn-Lucas_Hochmann_Rosa-0A66C2?style=for-the-badge&logo=linkedin">
  </a>
  <a href="#licença">
    <img src="https://img.shields.io/badge/License-MIT-2ea44f?style=for-the-badge">
  </a>
  <a href="https://en.wikipedia.org/wiki/C_(programming_language)">
    <img src="https://img.shields.io/badge/C-C11-00599C?style=for-the-badge&logo=c">
  </a>
</p>

> Um jogo de terror psicológico jogado inteiramente no terminal, feito por
> **Lucas Hochmann Rosa**. Licenciado sob MIT.

---

## 📌 Visão Geral

**Graveyard Shift** coloca você no lugar de
um(a) enfermeiro(a) no primeiro plantão noturno, sozinho(a), das 00:00 às
07:00. O relógio corre sozinho, em tempo real. Chamados vão chegando — um
idoso confuso vagando pelo corredor, uma criança com febre perigosa, uma
crise psiquiátrica, e muitos outros — e cada chamado é uma decisão real com
consequência real sobre a vida de alguém, não uma escolha de enfeite.

O jogo é inteiramente em **português**. Você pode **salvar no meio do plantão**
(tecla `0`) e **retomar depois** pela opção "Carregar Jogo" do menu.

---

## 🧠 Recursos

- **Um relógio de verdade**: o tempo avança sozinho, no ritmo simples de
  **1 minuto de jogo por segundo real**. Quando um chamado aparece, o relógio
  **para completamente** e só volta a andar depois que você decide.
- **20 cenários de paciente** escritos à mão, cada um com uma decisão real de
  tratamento entre três opções, chance de complicação, e uma consequência
  tardia caso você negligencie alguém.
- **Consequências que se acumulam**: repetir uma escolha evasiva (ignorar um
  chamado, não abrir uma porta) custa mais a cada vez; se engajar custa menos,
  mas engajamento também não é de graça pra sempre.
- **Frases ambientes de clima** espalhadas pelo turno, algumas só atmosféricas
  e outras que ainda elevam a sua aflição, para manter o alerta constante.
- **Três finais**, cada um alcançado por um padrão de jogo diferente, não um
  placar único: sobreviver até as 07:00, um ataque cardíaco por aflição não
  controlada, ou um colapso psiquiátrico que reinicia o plantão se você negar.
- **Salvamento em arquivo texto**: um único `save.txt`, escrito e lido linha a
  linha — simples de abrir e entender.
- **Arquitetura modular e fácil de explicar**: cada módulo tem um propósito
  claro, com identificadores em inglês e comentários em português.
- **Multiplataforma**: builda em Linux e Windows (via MinGW-w64).

---

## 🏗️ Arquitetura

```text
graveyard-shift-game/
│
├── docs/                    # Documentação de design
│   ├── GDD.md               # Documento de Design de Jogo
│   ├── ARCHITECTURE.md      # Arquitetura do sistema
│   └── STORY.md             # Documentação de narrativa
│
├── src/
│   ├── core/                # game.c (ciclo de vida), state.c (máquina de
│   │                         # estados), i18n.c (banco de textos em PT),
│   │                         # utils.c
│   ├── data/                # game_state.c (a struct GameData), player.c,
│   │                         # character.c (nome/nascimento/sexo), time.c
│   ├── narrative/           # story.c (flags de história)
│   ├── systems/             # gameplay.c (todo o loop de jogo e os três
│   │                         # finais), patients.c (os 20 cenários, dados
│   │                         # puros)
│   ├── save/                # save.c (salvar/carregar em arquivo texto)
│   ├── ui/                  # renderer.c (config. do terminal/console),
│   │                         # input.c (leitura de teclas), box.c (caixas
│   │                         # centralizadas)
│   └── main.c               # Conecta a máquina de estados
│
├── include/                 # Um header por arquivo fonte acima
├── README.md
├── CHANGELOG.md
├── LICENSE
├── Makefile                 # Caminho de build principal
└── CMakeLists.txt           # Alternativa opcional
```

---

## ⚙️ Requisitos

- Um compilador C11: **GCC** (via MinGW-w64 no Windows).
- `make`
- Um terminal com suporte a cores ANSI/UTF-8 (qualquer terminal moderno serve).

---

## 🔧 Instalação

Há **um caminho de build principal por sistema operacional**.

### Windows (MSYS2 + MinGW-w64)

O jogo usa chamadas nativas do console do Windows (saída em UTF-8 e códigos
ANSI) que exigem o toolchain **MinGW-w64**. A causa mais comum de falha de
build no Windows é abrir o terminal *errado* do MSYS2.

1. Instale o [MSYS2](https://www.msys2.org/).
2. Abra o atalho **"MSYS2 MinGW64"** no Menu Iniciar.
   **Não use o terminal comum "MSYS2 MSYS"** — ele usa um compilador diferente,
   de emulação POSIX, que não está preparado para compilar o código específico
   de console do Windows deste projeto.
3. Instale o toolchain (uma vez só):
   ```bash
   pacman -Syu
   pacman -S mingw-w64-x86_64-gcc make
   ```
   (Se o `pacman -Syu` pedir, feche e abra o terminal MinGW64 de novo.)
4. Clone, compile e rode, de dentro desse mesmo terminal MinGW64:
   ```bash
   git clone https://github.com/lucas-hochmann-rosa/graveyard-shift-game.git
   cd graveyard-shift-game
   make
   ./graveyard_shift
   ```

**Windows Defender / SmartScreen**: um `.exe` recém-compilado e sem assinatura
pode ser bloqueado na primeira vez que você rodar. Se `./graveyard_shift` disser
"Permission denied" ou for bloqueado, confira **Segurança do Windows → Proteção
contra vírus e ameaças → Histórico de proteção**, ou adicione uma exclusão para
a pasta do projeto. Isso é normal para qualquer executável sem assinatura que
você mesmo compila, não é um bug do jogo.

### Linux

```bash
git clone https://github.com/lucas-hochmann-rosa/graveyard-shift-game.git
cd graveyard-shift-game
make
./graveyard_shift
```

> O `CMakeLists.txt` continua no projeto como alternativa, mas o caminho
> recomendado e testado é o `make` acima.

---

## ▶️ Controles

- **Teclas numéricas**: opções de menu e de diálogo.
- **Enter** (durante entrada de texto): confirma nome / data de nascimento.
- **`0`** (durante o plantão): salva o jogo na hora e continua.
- **Esc** (durante o plantão): abre o **menu de pausa**, com o relógio parado —
  continuar, salvar e continuar, salvar e sair, ou sair sem salvar.

Ao começar um plantão novo, o jogo mostra uma tela de **"Como jogar"** com essas
teclas antes de a história começar.

---

## 🎮 Como um Plantão Se Desenrola

- **00:00 às 07:00**, um relógio contínuo em tempo real (1 minuto de jogo por
  segundo). Durante uma decisão o relógio para, então você nunca é apressado no
  meio de uma escolha.
- Os chamados chegam no próprio ritmo, com um intervalo aleatório entre eles
  para nunca se empilharem. Alguns são só clima; a maioria é um dos 20 cenários
  de paciente, cada um exigindo uma decisão real de tratamento.
- **Aflição** é o medidor de pressão do jogo: sobe com maus resultados e
  negligência, alivia um pouco com bons resultados, e sua trajetória decide pra
  qual final você está indo.
- **Finais**:
  - **Sobreviver** — o relógio chega às 07:00.
  - **Ataque cardíaco** — a aflição chega a 100%.
  - **Colapso psiquiátrico** — uma sequência de escolhas evasivas dispara uma
    cena de despertar numa ala psiquiátrica real; aceitar encerra o plantão
    ali; negar reinicia o plantão silenciosamente, com um sinal sutil de que
    isso já aconteceu antes.
- Ao chegar a um dos três finais, o `save.txt` é apagado (aquela partida
  acabou). Sair pelo menu de pausa não é um final: o save sobrevive e pode ser
  retomado. A opção "Carregar Jogo" só aparece no menu principal quando existe
  um save no disco.

---

## 🧩 Detalhes do Projeto

| Campo | Valor |
| ----- | ----- |
| Linguagem | C11 |
| Sistema de build | Make (principal), CMake 3.10+ (opcional) |
| Plataformas | Linux, Windows (MinGW-w64) |
| Licença | MIT |
| Idioma do jogo | Português |
| Sistema de save | Arquivo texto único (`save.txt`) |

---

## 📚 Documentação

- [Documento de Design de Jogo](docs/GDD.md)
- [Documento de Arquitetura](docs/ARCHITECTURE.md)
- [Documento de História](docs/STORY.md)
- [Changelog](CHANGELOG.md)

---

## 📄 Licença

Licenciado sob MIT. Você pode usar, modificar e distribuir este projeto
mantendo o aviso de direitos autorais e creditando **Lucas Hochmann Rosa**.

---

## 👨‍💻 Autor

**Lucas Hochmann Rosa**

- Repositório: https://github.com/lucas-hochmann-rosa/graveyard-shift-game
- GitHub: https://github.com/lucas-hochmann-rosa
- LinkedIn: https://www.linkedin.com/in/lucas-hochmann-rosa
- Licença: MIT
