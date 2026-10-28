// abb.c - ABB comum, so pra comparacao
// fiz tudo iterativo porque com 100 mil IDs em ordem a recursao estoura a pilha
// Thaliton do Nascimento Silva
#include <stdlib.h>
#include "abb.h"

struct NO_ABB {
    int info;
    struct NO_ABB *esq, *dir;
};

ArvABB *cria_ArvABB(void) {
    ArvABB *raiz = (ArvABB *) malloc(sizeof(ArvABB));
    if (raiz != NULL) *raiz = NULL;
    return raiz;
}

void libera_ArvABB(ArvABB *raiz) {
    if (raiz == NULL) return;
    // libera sem recursao: vai girando pra direita ate virar uma lista
    struct NO_ABB *p = *raiz;
    while (p != NULL) {
        if (p->esq != NULL) {
            struct NO_ABB *e = p->esq;
            p->esq = e->dir;
            e->dir = p;
            p = e;
        } else {
            struct NO_ABB *d = p->dir;
            free(p);
            p = d;
        }
    }
    free(raiz);
}

int insere_ArvABB(ArvABB *raiz, int valor) {
    if (raiz == NULL) return 0;
    struct NO_ABB **p = raiz;
    while (*p != NULL) {
        if (valor == (*p)->info) return 0;
        p = (valor < (*p)->info) ? &(*p)->esq : &(*p)->dir;
    }
    struct NO_ABB *novo = (struct NO_ABB *) malloc(sizeof(struct NO_ABB));
    if (novo == NULL) return 0;
    novo->info = valor;
    novo->esq = novo->dir = NULL;
    *p = novo;
    return 1;
}

int consulta_ArvABB(ArvABB *raiz, int valor, long *comparacoes) {
    struct NO_ABB *p = (raiz == NULL) ? NULL : *raiz;
    while (p != NULL) {
        if (comparacoes) (*comparacoes)++;
        if (valor == p->info) return 1;
        p = (valor < p->info) ? p->esq : p->dir;
    }
    return 0;
}

// altura por niveis (com fila), tambem sem recursao
int altura_ArvABB(ArvABB *raiz) {
    if (raiz == NULL || *raiz == NULL) return -1;
    int cap = 1024, ini = 0, fim = 0, h = -1;
    struct NO_ABB **fila = (struct NO_ABB **) malloc(cap * sizeof(*fila));
    if (fila == NULL) return -1;
    fila[fim++] = *raiz;
    while (ini < fim) {
        int nivelFim = fim;
        h++;
        while (ini < nivelFim) {
            struct NO_ABB *n = fila[ini++];
            struct NO_ABB *filhos[2] = { n->esq, n->dir };
            for (int k = 0; k < 2; k++) {
                if (filhos[k] == NULL) continue;
                if (fim == cap) {
                    // reaproveita o comeco da fila antes de aumentar
                    int usados = fim - ini;
                    for (int i = 0; i < usados; i++) fila[i] = fila[ini + i];
                    nivelFim -= ini; fim = usados; ini = 0;
                    if (fim == cap) {
                        cap *= 2;
                        struct NO_ABB **nf = (struct NO_ABB **) realloc(fila, cap * sizeof(*fila));
                        if (nf == NULL) { free(fila); return -1; }
                        fila = nf;
                    }
                }
                fila[fim++] = filhos[k];
            }
        }
    }
    free(fila);
    return h;
}
