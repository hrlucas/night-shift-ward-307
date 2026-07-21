# Makefile for Graveyard Shift

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -Iinclude
TARGET = graveyard_shift
SRCDIR = src
OBJDIR = build
SOURCES = $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Alvo padrão
all: $(TARGET)

# Cria os diretórios de build (um por subpasta de src/)
$(OBJDIR):
	mkdir -p $(OBJDIR)/core $(OBJDIR)/data $(OBJDIR)/systems $(OBJDIR)/ui $(OBJDIR)/narrative $(OBJDIR)/save

# Linka os objetos no executável
$(TARGET): $(OBJECTS) | $(OBJDIR)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

# Compila cada arquivo-fonte
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpa os artefatos de build
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Roda o jogo
run: $(TARGET)
	./$(TARGET)

# Instala
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Desinstala
uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean run install uninstall
