// avl.c - arvore AVL
// EDB II (IMD0039) - Thaliton do Nascimento Silva
//
// fb = altura(esq) - altura(dir). altura de NULL = -1, folha = 0
// se o fb chegar em +2 ou -2 a arvore gira:
//   LL -> simples a direita, RR -> simples a esquerda
//   LR e RL -> rotacao dupla
// ID repetido nao entra, e remover/buscar algo que nao existe so retorna 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "avl.h"

struct NO_AVL {
    int info;
    int alt;               // altura da subarvore
    struct NO_AVL *esq;
    struct NO_AVL *dir;
};

static EstatAVL est;   // contadores (pra analise)

// ---- auxiliares ----

static int alt_NO(struct NO_AVL *no) {
    if (no == NULL)
        return -1;
    else
        return no->alt;
}

// fb com sinal (sem o abs) porque o sinal diz pra que lado ta pendendo
static int fatorBalanceamento_NO(struct NO_AVL *no) {
    return alt_NO(no->esq) - alt_NO(no->dir);
}

static int maior(int x, int y) { return (x > y) ? x : y; }

static void atualizaAltura(struct NO_AVL *no) {
    no->alt = maior(alt_NO(no->esq), alt_NO(no->dir)) + 1;
}

// ---- rotacoes ----

// LL: rotacao simples a direita (B sobe, A vira filho direito dele)
static void RotacaoLL(ArvAVL *A) {
    struct NO_AVL *B = (*A)->esq;
    (*A)->esq = B->dir;
    B->dir = *A;
    atualizaAltura(*A);   // A desceu, entao atualiza ele primeiro
    atualizaAltura(B);
    *A = B;
    est.rotLL++;
}

// RR: rotacao simples a esquerda
static void RotacaoRR(ArvAVL *no) {
    struct NO_AVL *novoTopo = (*no)->dir;   // filho da direita sobe
    struct NO_AVL *temp = novoTopo->esq;    // guarda a subarvore do meio
    novoTopo->esq = *no;
    (*no)->dir = temp;                      // a do meio vai pra direita do no
    atualizaAltura(*no);
    atualizaAltura(novoTopo);
    *no = novoTopo;
    est.rotRR++;
}

// LR: dupla a direita (primeiro gira o filho pra esquerda, depois o no pra direita)
static void RotacaoLR(ArvAVL *A) {
    RotacaoRR(&(*A)->esq);
    RotacaoLL(A);
    est.rotRR--; est.rotLL--;   // conta como 1 dupla e nao como 2 simples
    est.rotLR++;
}

// RL: dupla a esquerda (o contrario da LR)
static void RotacaoRL(ArvAVL *A) {
    RotacaoLL(&(*A)->dir);
    RotacaoRR(A);
    est.rotRR--; est.rotLL--;
    est.rotRL++;
}

// escolhe a rotacao: sinal igual no no e no filho = simples, sinal diferente = dupla.
// fb do filho = 0 so acontece na remocao, ai a simples resolve
static void balanceia(ArvAVL *raiz) {
    int fb = fatorBalanceamento_NO(*raiz);
    if (fb >= 2) {
        if (fatorBalanceamento_NO((*raiz)->esq) >= 0) RotacaoLL(raiz);
        else                                          RotacaoLR(raiz);
    } else if (fb <= -2) {
        if (fatorBalanceamento_NO((*raiz)->dir) <= 0) RotacaoRR(raiz);
        else                                          RotacaoRL(raiz);
    }
}

// ---- criar e liberar ----
ArvAVL *cria_ArvAVL(void) {
    ArvAVL *raiz = (ArvAVL *) malloc(sizeof(ArvAVL));
    if (raiz != NULL) *raiz = NULL;
    return raiz;
}

static void libera_NO(struct NO_AVL *no) {
    if (no == NULL) return;
    libera_NO(no->esq);
    libera_NO(no->dir);
    free(no);
}

void libera_ArvAVL(ArvAVL *raiz) {
    if (raiz == NULL) return;
    libera_NO(*raiz);
    free(raiz);
}

// ---- insercao ----
// insere igual na ABB e na volta da recursao atualiza a altura e balanceia
static int insere_NO(ArvAVL *raiz, int valor) {
    int res;
    if (*raiz == NULL) {                    // chegou no lugar: cria a folha
        struct NO_AVL *novo = (struct NO_AVL *) malloc(sizeof(struct NO_AVL));
        if (novo == NULL) return 0;
        novo->info = valor;
        novo->alt = 0;
        novo->esq = novo->dir = NULL;
        *raiz = novo;
        return 1;
    }
    struct NO_AVL *atual = *raiz;
    if (valor < atual->info)
        res = insere_NO(&atual->esq, valor);
    else if (valor > atual->info)
        res = insere_NO(&atual->dir, valor);
    else
        return 0;                           // repetido: nao insere

    if (res == 1) {
        atualizaAltura(*raiz);
        balanceia(raiz);                    // na insercao no maximo 1 rotacao resolve
    }
    return res;
}

int insere_ArvAVL(ArvAVL *raiz, int valor) {
    if (raiz == NULL) return 0;
    return insere_NO(raiz, valor);
}

// ---- remocao ----
// remove igual na ABB e balanceia na volta (pode girar em varios niveis)
static struct NO_AVL *procuraMenor(struct NO_AVL *atual) {
    while (atual->esq != NULL) atual = atual->esq;
    return atual;
}

static int remove_NO(ArvAVL *raiz, int valor) {
    if (*raiz == NULL) return 0;            // nao achou
    int res;
    if (valor < (*raiz)->info) {
        res = remove_NO(&(*raiz)->esq, valor);
    } else if (valor > (*raiz)->info) {
        res = remove_NO(&(*raiz)->dir, valor);
    } else {
        struct NO_AVL *velho = *raiz;
        if (velho->esq == NULL || velho->dir == NULL) {
            // folha ou 1 filho: o filho sobe no lugar
            *raiz = (velho->esq != NULL) ? velho->esq : velho->dir;
            free(velho);
            return 1;
        }
        // 2 filhos: copia o sucessor (menor da direita) e remove ele
        struct NO_AVL *suc = procuraMenor(velho->dir);
        velho->info = suc->info;
        res = remove_NO(&velho->dir, suc->info);
    }
    if (res == 1 && *raiz != NULL) {
        atualizaAltura(*raiz);
        balanceia(raiz);
    }
    return res;
}

int remove_ArvAVL(ArvAVL *raiz, int valor) {
    if (raiz == NULL) return 0;
    return remove_NO(raiz, valor);
}

// ---- busca ----
int consulta_ArvAVL(ArvAVL *raiz, int valor) {
    if (raiz == NULL) return 0;
    struct NO_AVL *atual = *raiz;
    while (atual != NULL) {
        est.comparacoes++;
        if (valor == atual->info) return 1;
        atual = (valor > atual->info) ? atual->dir : atual->esq;
    }
    return 0;
}

// ---- outras consultas ----
int altura_ArvAVL(ArvAVL *raiz) {
    if (raiz == NULL) return -1;
    return alt_NO(*raiz);
}

static int conta(struct NO_AVL *no) {
    return no == NULL ? 0 : 1 + conta(no->esq) + conta(no->dir);
}
int totalNO_ArvAVL(ArvAVL *raiz) { return raiz == NULL ? 0 : conta(*raiz); }

int estaVazia_ArvAVL(ArvAVL *raiz) { return raiz == NULL || *raiz == NULL; }

// em ordem (esq, raiz, dir) sai em ordem crescente
static void emOrdem_NO(struct NO_AVL *no, int *primeiro) {
    if (no == NULL) return;
    emOrdem_NO(no->esq, primeiro);
    printf(*primeiro ? "%d" : " %d", no->info);
    *primeiro = 0;
    emOrdem_NO(no->dir, primeiro);
}
void emOrdem_ArvAVL(ArvAVL *raiz) {
    int primeiro = 1;
    if (raiz == NULL || *raiz == NULL) { printf("(árvore vazia)\n"); return; }
    emOrdem_NO(*raiz, &primeiro);
    printf("\n");
}

static void emOrdemVet(struct NO_AVL *no, int *v, int max, int *n) {
    if (no == NULL) return;
    emOrdemVet(no->esq, v, max, n);
    if (*n < max) v[(*n)++] = no->info;
    emOrdemVet(no->dir, v, max, n);
}
int emOrdemVetor_ArvAVL(ArvAVL *raiz, int *v, int max) {
    int n = 0;
    if (raiz != NULL) emOrdemVet(*raiz, v, max, &n);
    return n;
}

// pre-ordem, usada nos testes pra conferir o formato da arvore
static void preOrdemVet(struct NO_AVL *no, int *v, int max, int *n) {
    if (no == NULL) return;
    if (*n < max) v[(*n)++] = no->info;
    preOrdemVet(no->esq, v, max, n);
    preOrdemVet(no->dir, v, max, n);
}
int preOrdemVetor_ArvAVL(ArvAVL *raiz, int *v, int max) {
    int n = 0;
    if (raiz != NULL) preOrdemVet(*raiz, v, max, &n);
    return n;
}

// desenha a arvore deitada (direita em cima), com altura e fb de cada no
static void imprime_NO(struct NO_AVL *no, char *prefixo, int ehDireita, int ehRaiz) {
    if (no == NULL) return;
    char novo[1024];
    size_t len = strlen(prefixo);
    if (len > sizeof(novo) - 8) return;

    snprintf(novo, sizeof(novo), "%s%s", prefixo, ehRaiz ? "" : (ehDireita ? "    " : "|   "));
    imprime_NO(no->dir, novo, 1, 0);

    int fb = fatorBalanceamento_NO(no);
    printf("%s%s%d (h=%d, fb=%s%d)\n", prefixo, ehRaiz ? "" : (ehDireita ? ".-- " : "`-- "),
           no->info, no->alt, fb > 0 ? "+" : "", fb);

    snprintf(novo, sizeof(novo), "%s%s", prefixo, ehRaiz ? "" : (ehDireita ? "|   " : "    "));
    imprime_NO(no->esq, novo, 0, 0);
}
void imprime_ArvAVL(ArvAVL *raiz) {
    char prefixo[1024] = "";
    if (raiz == NULL || *raiz == NULL) { printf("(árvore vazia)\n"); return; }
    imprime_NO(*raiz, prefixo, 0, 1);
}

// ---- verificacao das propriedades ----
static int verifica_NO(struct NO_AVL *no, long min, long max, char *msg, int tam) {
    if (no == NULL) return -1;
    if (no->info <= min || no->info >= max) {
        snprintf(msg, tam, "ordem de ABB violada no nó %d", no->info);
        return INT_MIN;
    }
    int he = verifica_NO(no->esq, min, no->info, msg, tam);
    if (he == INT_MIN) return INT_MIN;
    int hd = verifica_NO(no->dir, no->info, max, msg, tam);
    if (hd == INT_MIN) return INT_MIN;
    int h = maior(he, hd) + 1;
    if (h != no->alt) {
        snprintf(msg, tam, "altura armazenada errada no nó %d (%d, real %d)", no->info, no->alt, h);
        return INT_MIN;
    }
    if (he - hd > 1 || he - hd < -1) {
        snprintf(msg, tam, "nó %d com fb = %+d (fora de {-1,0,+1})", no->info, he - hd);
        return INT_MIN;
    }
    return h;
}

int verifica_ArvAVL(ArvAVL *raiz, char *msg, int tam) {
    snprintf(msg, tam, "OK: ordem de ABB, alturas e fb em {-1,0,+1} em todos os nós");
    if (raiz == NULL) return 1;
    return verifica_NO(*raiz, LONG_MIN, LONG_MAX, msg, tam) != INT_MIN;
}

EstatAVL estat_ArvAVL(void) { return est; }
void zeraEstat_ArvAVL(void) { memset(&est, 0, sizeof(est)); }
long totalRotacoes_ArvAVL(void) {
    return est.rotLL + est.rotRR + 2 * (est.rotLR + est.rotRL);
}
