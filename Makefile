# Makefile - Amazon Engineering Challenge (EDB II / IMD0039)
# Autor: Thaliton do Nascimento Silva
#
#   make            compila os quatro programas em bin/
#   make testes     compila e roda a bateria de testes
#   make benchmark  compila e roda a análise comparativa (grava resultados/)
#   make simulacao  roda a simulação da Black Friday (série pequena + projeção)
#   make demo       roda os roteiros de exemplos/ e grava os logs em resultados/
#   make clean      apaga bin/

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c99
SRC     = src
BIN     = bin

ARVORES = $(SRC)/avl.c $(SRC)/rubronegra.c
HEADERS = $(SRC)/avl.h $(SRC)/rubronegra.h $(SRC)/abb.h $(SRC)/plataforma.h

all: $(BIN)/amazon_arvores $(BIN)/testes $(BIN)/benchmark $(BIN)/simulacao

$(BIN):
	mkdir -p $(BIN)

$(BIN)/amazon_arvores: $(SRC)/main.c $(ARVORES) $(HEADERS) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/main.c $(ARVORES)

$(BIN)/testes: $(SRC)/testes.c $(ARVORES) $(HEADERS) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/testes.c $(ARVORES)

$(BIN)/benchmark: $(SRC)/benchmark.c $(ARVORES) $(SRC)/abb.c $(HEADERS) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/benchmark.c $(ARVORES) $(SRC)/abb.c

$(BIN)/simulacao: $(SRC)/simulacao.c $(ARVORES) $(SRC)/abb.c $(HEADERS) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/simulacao.c $(ARVORES) $(SRC)/abb.c -lm

testes: $(BIN)/testes
	./$(BIN)/testes

benchmark: $(BIN)/benchmark
	mkdir -p resultados
	./$(BIN)/benchmark 5 | tee resultados/benchmark_saida.txt

simulacao: $(BIN)/simulacao
	mkdir -p resultados
	./$(BIN)/simulacao | tee resultados/simulacao_serie_saida.txt

demo: $(BIN)/amazon_arvores
	mkdir -p resultados
	for r in avl rubronegra slide_aula17 slide_aula18; do \
		./$(BIN)/amazon_arvores --sem-cor --eco < exemplos/roteiro_$$r.txt > resultados/demo_$$r.txt; \
	done

clean:
	rm -f $(BIN)/amazon_arvores $(BIN)/testes $(BIN)/benchmark $(BIN)/simulacao $(BIN)/*.exe

.PHONY: all testes benchmark simulacao demo clean
