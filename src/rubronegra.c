// rubronegra.c - arvore rubro-negra caida para a esquerda (LLRB)
// EDB II (IMD0039) - Thaliton do Nascimento Silva
//
// escolhi a LLRB porque a insercao e a remocao ficam mais simples e
// continua valendo tudo da rubro-negra normal:
//   - todo no e vermelho ou preto, a raiz e preta, NULL conta como preto
//   - no vermelho nao tem filho vermelho
//   - todo caminho ate o NULL tem a mesma qtd de pretos
//   - e na LLRB o vermelho sempre fica a esquerda
// a remocao (move2EsqRED, move2DirRED, removerMenor) segue o codigo do
// prof. Andre Backes, que e baseado no Sedgewick.
//
// ID repetido nao entra. remover algo que nao existe retorna 0 sem mexer na
// arvore (por isso confere antes, ja que a remocao muda os nos na descida)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "rubronegra.h"

struct NO_RN {
    int info;
    struct NO_RN *esq;
    struct NO_RN *dir;
    int cor;
};

static EstatRN est;

// ---- auxiliares ----

// NULL e preto
static int cor(struct NO_RN *H) {
    if (H == NULL)
        return BLACK;
    else
        return H->cor;
}

// inverte a cor do pai e dos dois filhos (nao muda a estrutura)
static void trocaCor(struct NO_RN *H) {
    H->cor = !H->cor;
    if (H->esq != NULL)
        H->esq->cor = !H->esq->cor;
    if (H->dir != NULL)
        H->dir->cor = !H->dir->cor;
    est.trocasCor++;
}

// rotacao a esquerda: o filho direito sobe, herda a cor do A e o A fica vermelho
static struct NO_RN *rotacionaEsquerda(struct NO_RN *A) {
    struct NO_RN *B = A->dir;
    A->dir = B->esq;
    B->esq = A;
    B->cor = A->cor;
    A->cor = RED;
    est.rotEsq++;
    return B;
}

// rotacao a direita (o contrario da esquerda)
static struct NO_RN *rotacionaDireita(struct NO_RN *A) {
    struct NO_RN *B = A->esq;
    A->esq = B->dir;
    B->dir = A;
    B->cor = A->cor;
    A->cor = RED;
    est.rotDir++;
    return B;
}

// usado na descida da remocao pra garantir que o lado esquerdo tenha um vermelho
static struct NO_RN *move2EsqRED(struct NO_RN *H) {
    trocaCor(H);
    if (cor(H->dir->esq) == RED) {
        H->dir = rotacionaDireita(H->dir);
        H = rotacionaEsquerda(H);
        trocaCor(H);
    }
    return H;
}

// mesma coisa pro lado direito
static struct NO_RN *move2DirRED(struct NO_RN *H) {
    trocaCor(H);
    if (cor(H->esq->esq) == RED) {
        H = rotacionaDireita(H);
        trocaCor(H);
    }
    return H;
}

// arruma na volta da recursao:
//   direito vermelho e esquerdo nao -> gira a esquerda
//   esquerdo e neto esquerdo vermelhos -> gira a direita
//   os dois filhos vermelhos -> troca a cor
// pode acontecer mais de uma correcao no mesmo no
static struct NO_RN *balancear(struct NO_RN *H) {
    if (cor(H->dir) == RED && cor(H->esq) == BLACK)
        H = rotacionaEsquerda(H);
    if (cor(H->esq) == RED && cor(H->esq->esq) == RED)
        H = rotacionaDireita(H);
    if (cor(H->esq) == RED && cor(H->dir) == RED)
        trocaCor(H);
    return H;
}

// versao da remocao: aqui o direito vermelho sempre gira, senao sobra
// vermelho do lado direito depois do move2EsqRED
static struct NO_RN *balancearRemocao(struct NO_RN *H) {
    if (cor(H->dir) == RED)
        H = rotacionaEsquerda(H);
    if (H->esq != NULL && cor(H->esq) == RED && cor(H->esq->esq) == RED)
        H = rotacionaDireita(H);
    if (cor(H->esq) == RED && cor(H->dir) == RED)
        trocaCor(H);
    return H;
}

// ---- criar e liberar ----
ArvLLRB *cria_ArvLLRB(void) {
    ArvLLRB *raiz = (ArvLLRB *) malloc(sizeof(ArvLLRB));
    if (raiz != NULL) *raiz = NULL;
    return raiz;
}

static void libera_NO(struct NO_RN *no) {
    if (no == NULL) return;
    libera_NO(no->esq);
    libera_NO(no->dir);
    free(no);
}

void libera_ArvLLRB(ArvLLRB *raiz) {
    if (raiz == NULL) return;
    libera_NO(*raiz);
    free(raiz);
}

// ---- insercao ----
// insere igual na ABB (sempre vermelho) e corrige na volta
static struct NO_RN *insereNO(struct NO_RN *H, int valor, int *resp) {
    if (H == NULL) {
        struct NO_RN *novo = (struct NO_RN *) malloc(sizeof(struct NO_RN));
        if (novo == NULL) { *resp = 0; return NULL; }
        novo->info = valor;
        novo->cor = RED;                    // no novo e sempre vermelho
        novo->esq = novo->dir = NULL;
        *resp = 1;
        return novo;
    }
    if (valor == H->info)
        *resp = 0;                          // repetido: nao insere
    else if (valor < H->info)
        H->esq = insereNO(H->esq, valor, resp);
    else
        H->dir = insereNO(H->dir, valor, resp);

    return balancear(H);
}

int insere_ArvLLRB(ArvLLRB *raiz, int valor) {
    int resp = 0;
    if (raiz == NULL) return 0;
    *raiz = insereNO(*raiz, valor, &resp);
    if (*raiz != NULL)
        (*raiz)->cor = BLACK;               // no fim a raiz fica preta
    return resp;
}

// ---- remocao ----
// corrige na descida e na volta da recursao
static struct NO_RN *procuraMenor(struct NO_RN *atual) {
    while (atual->esq != NULL) atual = atual->esq;
    return atual;
}

static struct NO_RN *removerMenor(struct NO_RN *H) {
    if (H->esq == NULL) {
        free(H);
        return NULL;
    }
    if (cor(H->esq) == BLACK && cor(H->esq->esq) == BLACK)
        H = move2EsqRED(H);                 // arruma na descida
    H->esq = removerMenor(H->esq);
    return balancearRemocao(H);             // arruma na volta
}

static struct NO_RN *remove_NO(struct NO_RN *H, int valor) {
    if (valor < H->info) {
        if (cor(H->esq) == BLACK && cor(H->esq->esq) == BLACK)
            H = move2EsqRED(H);
        H->esq = remove_NO(H->esq, valor);
    } else {
        if (cor(H->esq) == RED)
            H = rotacionaDireita(H);
        if (valor == H->info && H->dir == NULL) {   // e uma folha
            free(H);
            return NULL;
        }
        if (cor(H->dir) == BLACK && cor(H->dir->esq) == BLACK)
            H = move2DirRED(H);
        if (valor == H->info) {                     // no com filhos: troca pelo sucessor
            struct NO_RN *x = procuraMenor(H->dir);
            H->info = x->info;
            H->dir = removerMenor(H->dir);
        } else {
            H->dir = remove_NO(H->dir, valor);
        }
    }
    return balancearRemocao(H);
}

int remove_ArvLLRB(ArvLLRB *raiz, int valor) {
    if (raiz == NULL || *raiz == NULL) return 0;
    long cmp = est.comparacoes;             // nao conta essa busca nas comparacoes
    int existe = consulta_ArvLLRB(raiz, valor);
    est.comparacoes = cmp;
    if (!existe) return 0;                  // so remove se existir

    *raiz = remove_NO(*raiz, valor);
    if (*raiz != NULL)
        (*raiz)->cor = BLACK;
    return 1;
}

// ---- busca ----
int consulta_ArvLLRB(ArvLLRB *raiz, int valor) {
    if (raiz == NULL) return 0;
    struct NO_RN *atual = *raiz;
    while (atual != NULL) {
        est.comparacoes++;
        if (valor == atual->info) return 1;
        atual = (valor > atual->info) ? atual->dir : atual->esq;
    }
    return 0;
}

// ---- outras consultas ----
static int altura_NO(struct NO_RN *no) {
    if (no == NULL) return -1;
    int he = altura_NO(no->esq), hd = altura_NO(no->dir);
    return (he > hd ? he : hd) + 1;
}
int altura_ArvLLRB(ArvLLRB *raiz) { return raiz == NULL ? -1 : altura_NO(*raiz); }

int alturaNegra_ArvLLRB(ArvLLRB *raiz) {
    int bh = 0;
    if (raiz == NULL) return 0;
    for (struct NO_RN *p = *raiz; p != NULL; p = p->esq)
        if (p->cor == BLACK) bh++;
    return bh;
}

static int conta(struct NO_RN *no) {
    return no == NULL ? 0 : 1 + conta(no->esq) + conta(no->dir);
}
int totalNO_ArvLLRB(ArvLLRB *raiz) { return raiz == NULL ? 0 : conta(*raiz); }
int estaVazia_ArvLLRB(ArvLLRB *raiz) { return raiz == NULL || *raiz == NULL; }

static void emOrdem_NO(struct NO_RN *no, int *primeiro) {
    if (no == NULL) return;
    emOrdem_NO(no->esq, primeiro);
    printf(*primeiro ? "%d" : " %d", no->info);
    *primeiro = 0;
    emOrdem_NO(no->dir, primeiro);
}
void emOrdem_ArvLLRB(ArvLLRB *raiz) {
    int primeiro = 1;
    if (raiz == NULL || *raiz == NULL) { printf("(árvore vazia)\n"); return; }
    emOrdem_NO(*raiz, &primeiro);
    printf("\n");
}

static void emOrdemVet(struct NO_RN *no, int *v, int max, int *n) {
    if (no == NULL) return;
    emOrdemVet(no->esq, v, max, n);
    if (*n < max) v[(*n)++] = no->info;
    emOrdemVet(no->dir, v, max, n);
}
int emOrdemVetor_ArvLLRB(ArvLLRB *raiz, int *v, int max) {
    int n = 0;
    if (raiz != NULL) emOrdemVet(*raiz, v, max, &n);
    return n;
}

// pre-ordem com as cores, usada nos testes
static void preOrdemVet(struct NO_RN *no, int *v, int *cores, int max, int *n) {
    if (no == NULL) return;
    if (*n < max) { v[*n] = no->info; if (cores) cores[*n] = no->cor; (*n)++; }
    preOrdemVet(no->esq, v, cores, max, n);
    preOrdemVet(no->dir, v, cores, max, n);
}
int preOrdemVetor_ArvLLRB(ArvLLRB *raiz, int *v, int *cores, int max) {
    int n = 0;
    if (raiz != NULL) preOrdemVet(*raiz, v, cores, max, &n);
    return n;
}

// desenha a arvore deitada. cada no aparece como 10[P] ou 5[V]
// com usarCores = 1 o numero sai colorido
static void imprime_NO(struct NO_RN *no, char *prefixo, int ehDireita, int ehRaiz, int usarCores) {
    if (no == NULL) return;
    char novo[1024];
    if (strlen(prefixo) > sizeof(novo) - 8) return;

    snprintf(novo, sizeof(novo), "%s%s", prefixo, ehRaiz ? "" : (ehDireita ? "    " : "|   "));
    imprime_NO(no->dir, novo, 1, 0, usarCores);

    const char *ini = "", *fim = "";
    if (usarCores) {
        ini = (no->cor == RED) ? "\033[1;31m" : "\033[1;37;40m";
        fim = "\033[0m";
    }
    printf("%s%s%s%d[%s]%s\n", prefixo, ehRaiz ? "" : (ehDireita ? ".-- " : "`-- "),
           ini, no->info, no->cor == RED ? "V" : "P", fim);

    snprintf(novo, sizeof(novo), "%s%s", prefixo, ehRaiz ? "" : (ehDireita ? "|   " : "    "));
    imprime_NO(no->esq, novo, 0, 0, usarCores);
}
void imprime_ArvLLRB(ArvLLRB *raiz, int usarCores) {
    char prefixo[1024] = "";
    if (raiz == NULL || *raiz == NULL) { printf("(árvore vazia)\n"); return; }
    imprime_NO(*raiz, prefixo, 0, 1, usarCores);
    printf("Legenda: [P] = preto, [V] = vermelho\n");
}

// ---- verificacao das propriedades ----
// retorna a altura negra ou -1 se alguma regra foi quebrada
static int verifica_NO(struct NO_RN *no, long min, long max, char *msg, int tam) {
    if (no == NULL) return 1;   // NULL conta como preto
    if (no->info <= min || no->info >= max) {
        snprintf(msg, tam, "ordem de ABB violada no nó %d", no->info);
        return -1;
    }
    if (no->cor != RED && no->cor != BLACK) {
        snprintf(msg, tam, "nó %d sem cor válida (prop. 1)", no->info);
        return -1;
    }
    if (cor(no->dir) == RED) {
        snprintf(msg, tam, "ligação vermelha à direita em %d (regra LLRB)", no->info);
        return -1;
    }
    if (no->cor == RED && (cor(no->esq) == RED || cor(no->dir) == RED)) {
        snprintf(msg, tam, "vermelhos consecutivos a partir de %d (prop. 4)", no->info);
        return -1;
    }
    int be = verifica_NO(no->esq, min, no->info, msg, tam);
    if (be < 0) return -1;
    int bd = verifica_NO(no->dir, no->info, max, msg, tam);
    if (bd < 0) return -1;
    if (be != bd) {
        snprintf(msg, tam, "alturas negras diferentes em %d: %d x %d (prop. 5)", no->info, be, bd);
        return -1;
    }
    return be + (no->cor == BLACK ? 1 : 0);
}

int verifica_ArvLLRB(ArvLLRB *raiz, char *msg, int tam) {
    snprintf(msg, tam, "OK: raiz preta, sem vermelhos consecutivos, "
                       "vermelhos só à esquerda e mesma altura negra");
    if (raiz == NULL || *raiz == NULL) return 1;
    if ((*raiz)->cor != BLACK) {
        snprintf(msg, tam, "raiz %d não é preta (prop. 2)", (*raiz)->info);
        return 0;
    }
    return verifica_NO(*raiz, LONG_MIN, LONG_MAX, msg, tam) >= 0;
}

EstatRN estat_ArvLLRB(void) { return est; }
void zeraEstat_ArvLLRB(void) { memset(&est, 0, sizeof(est)); }
long totalRotacoes_ArvLLRB(void) { return est.rotEsq + est.rotDir; }
