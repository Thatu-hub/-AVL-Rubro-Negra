// testes.c - testes automaticos das duas arvores
// EDB II (IMD0039) - Thaliton do Nascimento Silva
//
// cada teste roda com os mesmos dados nas duas arvores e depois de cada
// insercao/remocao confere se as propriedades continuam valendo
//
// ./testes      so o resumo
// ./testes -v   mostra cada verificacao
#include "plataforma.h"   // tem que vir antes dos outros includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"
#include "rubronegra.h"

static int verbose = 0, total = 0, falhas = 0;

#define CHECK(cond, ...) do {                                        \
        total++;                                                     \
        if (!(cond)) { falhas++; printf("  [FALHOU] "); printf(__VA_ARGS__); printf("\n"); } \
        else if (verbose) { printf("  [ok] "); printf(__VA_ARGS__); printf("\n"); } \
    } while (0)

// ---- funcoes em comum ----
// assim cada teste e escrito uma vez so e roda nas duas arvores
typedef struct {
    const char *nome;
    void *(*cria)(void);
    void  (*libera)(void *);
    int   (*insere)(void *, int);
    int   (*remove)(void *, int);
    int   (*busca)(void *, int);
    int   (*altura)(void *);
    int   (*total)(void *);
    int   (*emOrdem)(void *, int *, int);
    int   (*verifica)(void *, char *, int);
    long  (*rotacoes)(void);
    void  (*zera)(void);
} Arvore;

static void *cAVL(void) { zeraEstat_ArvAVL(); return cria_ArvAVL(); }
static void  lAVL(void *t) { libera_ArvAVL((ArvAVL *) t); }
static int   iAVL(void *t, int v) { return insere_ArvAVL((ArvAVL *) t, v); }
static int   rAVL(void *t, int v) { return remove_ArvAVL((ArvAVL *) t, v); }
static int   bAVL(void *t, int v) { return consulta_ArvAVL((ArvAVL *) t, v); }
static int   hAVL(void *t) { return altura_ArvAVL((ArvAVL *) t); }
static int   nAVL(void *t) { return totalNO_ArvAVL((ArvAVL *) t); }
static int   oAVL(void *t, int *v, int m) { return emOrdemVetor_ArvAVL((ArvAVL *) t, v, m); }
static int   vAVL(void *t, char *m, int n) { return verifica_ArvAVL((ArvAVL *) t, m, n); }

static void *cRN(void) { zeraEstat_ArvLLRB(); return cria_ArvLLRB(); }
static void  lRN(void *t) { libera_ArvLLRB((ArvLLRB *) t); }
static int   iRN(void *t, int v) { return insere_ArvLLRB((ArvLLRB *) t, v); }
static int   rRN(void *t, int v) { return remove_ArvLLRB((ArvLLRB *) t, v); }
static int   bRN(void *t, int v) { return consulta_ArvLLRB((ArvLLRB *) t, v); }
static int   hRN(void *t) { return altura_ArvLLRB((ArvLLRB *) t); }
static int   nRN(void *t) { return totalNO_ArvLLRB((ArvLLRB *) t); }
static int   oRN(void *t, int *v, int m) { return emOrdemVetor_ArvLLRB((ArvLLRB *) t, v, m); }
static int   vRN(void *t, char *m, int n) { return verifica_ArvLLRB((ArvLLRB *) t, m, n); }

static const Arvore ARVORES[2] = {
    { "AVL", cAVL, lAVL, iAVL, rAVL, bAVL, hAVL, nAVL, oAVL, vAVL, totalRotacoes_ArvAVL, zeraEstat_ArvAVL },
    { "Rubro-Negra (LLRB)", cRN, lRN, iRN, rRN, bRN, hRN, nRN, oRN, vRN, totalRotacoes_ArvLLRB, zeraEstat_ArvLLRB },
};

// ---- utilidades ----
static unsigned long semente = 2026;
static int aleatorio(int n) {            // gerador aleatorio simples, sempre da a mesma sequencia
    semente = semente * 6364136223846793005UL + 1442695040888963407UL;
    return (int) ((semente >> 33) % (unsigned long) n);
}
static void embaralha(int *v, int n) {
    for (int i = n - 1; i > 0; i--) { int j = aleatorio(i + 1); int t = v[i]; v[i] = v[j]; v[j] = t; }
}

static int emOrdemCorreta(const Arvore *A, void *t, int esperado) {
    int *v = (int *) malloc((esperado + 1) * sizeof(int));
    int n = A->emOrdem(t, v, esperado + 1), ok = (n == esperado);
    for (int i = 1; ok && i < n; i++) if (v[i - 1] >= v[i]) ok = 0;
    free(v);
    return ok;
}

static int valida(const Arvore *A, void *t, const char *quando) {
    char msg[256];
    int ok = A->verifica(t, msg, sizeof msg);
    if (!ok) printf("  [%s] propriedade violada %s: %s\n", A->nome, quando, msg);
    return ok;
}

// ---- cenarios ----
static void testeArvoreVazia(const Arvore *A) {
    void *t = A->cria();
    CHECK(A->altura(t) == -1, "%s: árvore vazia tem altura -1", A->nome);
    CHECK(A->total(t) == 0, "%s: árvore vazia tem 0 nós", A->nome);
    CHECK(A->busca(t, 10) == 0, "%s: busca em árvore vazia retorna 0", A->nome);
    CHECK(A->remove(t, 10) == 0, "%s: remoção em árvore vazia retorna 0", A->nome);
    CHECK(valida(A, t, "na árvore vazia"), "%s: árvore vazia é válida", A->nome);
    A->libera(t);
}

static void testeUmElemento(const Arvore *A) {
    void *t = A->cria();
    CHECK(A->insere(t, 42) == 1, "%s: insere 42 na árvore vazia", A->nome);
    CHECK(A->altura(t) == 0 && A->total(t) == 1, "%s: um elemento -> altura 0, 1 nó", A->nome);
    CHECK(valida(A, t, "com 1 elemento"), "%s: árvore com 1 elemento é válida", A->nome);
    CHECK(A->busca(t, 42) == 1, "%s: encontra 42", A->nome);
    CHECK(A->busca(t, 7) == 0, "%s: não encontra 7", A->nome);
    CHECK(A->remove(t, 7) == 0 && A->total(t) == 1, "%s: remover 7 (inexistente) não altera", A->nome);
    CHECK(A->remove(t, 42) == 1 && A->total(t) == 0, "%s: remove o único elemento", A->nome);
    CHECK(A->altura(t) == -1, "%s: árvore volta a ficar vazia", A->nome);
    A->libera(t);
}

static void testeDuplicados(const Arvore *A) {
    void *t = A->cria();
    int vals[] = {50, 30, 70, 30, 50, 70};
    int aceitos = 0;
    for (int i = 0; i < 6; i++) aceitos += A->insere(t, vals[i]);
    CHECK(aceitos == 3 && A->total(t) == 3, "%s: 3 de 6 aceitos (duplicados recusados)", A->nome);
    CHECK(valida(A, t, "após duplicados"), "%s: válida após tentativas de duplicação", A->nome);
    A->libera(t);
}

// insere em ordem crescente, decrescente e embaralhada, conferindo a cada passo,
// e depois busca IDs que existem e que nao existem
static void testeOrdem(const Arvore *A, const char *ordem, int *v, int n) {
    void *t = A->cria();
    int ok = 1;
    for (int i = 0; i < n; i++) {
        A->insere(t, v[i]);
        if (!valida(A, t, "durante a inserção")) { ok = 0; break; }
    }
    CHECK(ok, "%s: propriedades mantidas após CADA uma das %d inserções (%s)", A->nome, n, ordem);
    CHECK(A->total(t) == n, "%s: %d nós (%s)", A->nome, n, ordem);
    CHECK(emOrdemCorreta(A, t, n), "%s: percurso em ordem crescente (%s)", A->nome, ordem);

    int achou = 0, naoAchou = 0;
    for (int i = 0; i < n; i++) achou += A->busca(t, v[i]);
    for (int i = 0; i < n; i++) naoAchou += !A->busca(t, n * 10 + i);   // IDs que nao estao na arvore
    CHECK(achou == n, "%s: encontra os %d existentes (%s)", A->nome, n, ordem);
    CHECK(naoAchou == n, "%s: não encontra %d inexistentes (%s)", A->nome, n, ordem);
    if (verbose) printf("     -> altura final %d, rotações %ld\n", A->altura(t), A->rotacoes());
    A->libera(t);
}

// remocao com tipos diferentes de no (folha, 1 filho, 2 filhos) nesta arvore:
//            50
//        30       70
//      20  40   60   80
//     10              90
static void testeRemocoes(const Arvore *A) {
    int base[] = {50, 30, 70, 20, 40, 60, 80, 10, 90};
    int nb = 9;
    // os casos de filhos sao os da AVL/ABB acima; na LLRB o formato pode mudar por causa das cores
    struct { int valor; const char *caso; } casos[] = {
        {10, "folha"}, {20, "nó com 1 filho à esquerda"}, {80, "nó com 1 filho à direita"},
        {30, "nó com 2 filhos"}, {50, "raiz com 2 filhos"},
    };
    for (int c = 0; c < 5; c++) {
        void *t = A->cria();
        for (int i = 0; i < nb; i++) A->insere(t, base[i]);
        int ok = A->remove(t, casos[c].valor) == 1
              && !A->busca(t, casos[c].valor)
              && valida(A, t, "após remoção");
        CHECK(ok && A->total(t) == nb - 1 && emOrdemCorreta(A, t, nb - 1),
              "%s: remoção de %s (%d) mantém propriedades", A->nome, casos[c].caso, casos[c].valor);
        A->libera(t);
    }
    // remove um atras do outro conferindo sempre
    {
        void *t = A->cria();
        for (int i = 0; i < nb; i++) A->insere(t, base[i]);
        int ordem[] = {50, 30, 10, 90, 70, 20, 40, 60, 80}, ok = 1;
        for (int i = 0; i < nb; i++)
            if (A->remove(t, ordem[i]) != 1 || !valida(A, t, "remoções em sequência")) ok = 0;
        CHECK(ok && A->total(t) == 0, "%s: 9 remoções em sequência, válidas a cada passo", A->nome);
        A->libera(t);
    }
    void *t = A->cria();
    for (int i = 0; i < nb; i++) A->insere(t, base[i]);
    CHECK(A->remove(t, 55) == 0 && A->total(t) == nb && valida(A, t, "após remover inexistente"),
          "%s: remover 55 (inexistente) não altera a árvore", A->nome);
    A->libera(t);
}

// teste de estresse: operacoes aleatorias comparadas com um vetor que diz quem
// deveria estar na arvore. confere as propriedades de tempos em tempos
static void testeEstresse(const Arvore *A) {
    const int U = 3000, OPS = 40000;
    char *presente = (char *) calloc(U, 1);
    void *t = A->cria();
    int n = 0, erros = 0, invalida = 0;
    semente = 99;
    for (int i = 0; i < OPS; i++) {
        int op = aleatorio(3), x = aleatorio(U);
        if (op == 0) { int r = A->insere(t, x); if (r != !presente[x]) erros++; if (r) { presente[x] = 1; n++; } }
        else if (op == 1) { int r = A->remove(t, x); if (r != presente[x]) erros++; if (r) { presente[x] = 0; n--; } }
        else { if (A->busca(t, x) != presente[x]) erros++; }
        if (i % 250 == 0 && !valida(A, t, "no estresse")) invalida++;
    }
    CHECK(erros == 0, "%s: %d operações aleatórias batem com o gabarito", A->nome, OPS);
    CHECK(invalida == 0 && valida(A, t, "no fim do estresse"), "%s: propriedades válidas durante o estresse", A->nome);
    CHECK(A->total(t) == n, "%s: contagem final de nós confere (%d)", A->nome, n);
    // tira tudo em ordem aleatoria
    int *v = (int *) malloc(U * sizeof(int)), k = 0;
    for (int i = 0; i < U; i++) if (presente[i]) v[k++] = i;
    embaralha(v, k);
    int ok = 1;
    for (int i = 0; i < k; i++) { if (A->remove(t, v[i]) != 1) ok = 0; if (i % 50 == 0 && !valida(A, t, "esvaziando")) ok = 0; }
    CHECK(ok && A->total(t) == 0 && A->altura(t) == -1, "%s: remover todos deixa a árvore vazia", A->nome);
    free(v); free(presente);
    A->libera(t);
}

// ---- exemplos resolvidos em aula ----
static int vetIgual(const int *a, const int *b, int n) {
    for (int i = 0; i < n; i++) if (a[i] != b[i]) return 0;
    return 1;
}

static void testeExemplosDosSlides(void) {
    int pre[32], cores[32], n;

    // sequencia que degenera a ABB, na AVL fica: raiz 4; 2(1,3); 7(5(-,6), 9(8,10))
    {
        int seq[] = {1, 2, 3, 10, 4, 5, 9, 7, 8, 6};
        int esperado[] = {4, 2, 1, 3, 7, 5, 6, 9, 8, 10};
        ArvAVL *t = cria_ArvAVL();
        for (int i = 0; i < 10; i++) insere_ArvAVL(t, seq[i]);
        n = preOrdemVetor_ArvAVL(t, pre, 32);
        CHECK(n == 10 && vetIgual(pre, esperado, 10),
              "AVL: {1,2,3,10,4,5,9,7,8,6} gera a árvore do slide 22 da Aula 17");
        libera_ArvAVL(t);
    }
    // caso LR: arvore 50(30(20,40),60), insere 45, desbalanceia no 50
    // e fica 40(30(20,-),50(45,60))
    {
        int seq[] = {50, 30, 60, 20, 40, 45};
        int esperado[] = {40, 30, 20, 50, 45, 60};
        ArvAVL *t = cria_ArvAVL();
        zeraEstat_ArvAVL();
        for (int i = 0; i < 6; i++) insere_ArvAVL(t, seq[i]);
        n = preOrdemVetor_ArvAVL(t, pre, 32);
        CHECK(n == 6 && vetIgual(pre, esperado, 6) && estat_ArvAVL().rotLR == 1,
              "AVL: caso LR do slide (dupla à direita = esq. em B + dir. em A)");
        libera_ArvAVL(t);
    }
    // LLRB com 10 20 30 25 5 1 2. no fim: 20 preto na raiz, 5 vermelho a esquerda,
    // 2 preto com 1 vermelho, 10 preto, 30 preto com 25 vermelho
    {
        int seq[] = {10, 20, 30, 25, 5, 1, 2};
        int esperado[] = {20, 5, 2, 1, 10, 30, 25};
        int corEsp[]  = {BLACK, RED, BLACK, RED, BLACK, BLACK, RED};
        ArvLLRB *t = cria_ArvLLRB();
        for (int i = 0; i < 7; i++) insere_ArvLLRB(t, seq[i]);
        n = preOrdemVetor_ArvLLRB(t, pre, cores, 32);
        CHECK(n == 7 && vetIgual(pre, esperado, 7) && vetIgual(cores, corEsp, 7),
              "LLRB: 10,20,30,25,5,1,2 gera a árvore (e as cores) do slide 77 da Aula 18");
        libera_ArvLLRB(t);
    }
    // 30 20 10: gira a direita, troca a cor e a raiz fica preta -> 20(P) com 10(P) e 30(P)
    {
        int esperado[] = {20, 10, 30}, corEsp[] = {BLACK, BLACK, BLACK};
        ArvLLRB *t = cria_ArvLLRB();
        zeraEstat_ArvLLRB();
        insere_ArvLLRB(t, 30); insere_ArvLLRB(t, 20); insere_ArvLLRB(t, 10);
        n = preOrdemVetor_ArvLLRB(t, pre, cores, 32);
        EstatRN e = estat_ArvLLRB();
        CHECK(n == 3 && vetIgual(pre, esperado, 3) && vetIgual(cores, corEsp, 3)
              && e.rotDir == 1 && e.trocasCor == 1,
              "LLRB: Exercício 5 (30,20,10) = 1 rotação à direita + 1 troca de cor");
        libera_ArvLLRB(t);
    }
}

int main(int argc, char **argv) {
    prepara_console();
    if (argc > 1 && strcmp(argv[1], "-v") == 0) verbose = 1;
    const int N = 1000;
    int *cres = (int *) malloc(N * sizeof(int)), *decr = (int *) malloc(N * sizeof(int)),
        *alea = (int *) malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) { cres[i] = i + 1; decr[i] = N - i; alea[i] = i + 1; }
    semente = 2026;
    embaralha(alea, N);

    for (int a = 0; a < 2; a++) {
        const Arvore *A = &ARVORES[a];
        printf("\n=== %s ===\n", A->nome);
        int f0 = falhas, t0 = total;
        testeArvoreVazia(A);
        testeUmElemento(A);
        testeDuplicados(A);
        testeOrdem(A, "crescente", cres, N);
        testeOrdem(A, "decrescente", decr, N);
        testeOrdem(A, "não ordenada", alea, N);
        testeRemocoes(A);
        testeEstresse(A);
        printf("  %d/%d verificações OK\n", (total - t0) - (falhas - f0), total - t0);
    }
    printf("\n=== Exemplos resolvidos dos slides ===\n");
    int f0 = falhas, t0 = total;
    testeExemplosDosSlides();
    printf("  %d/%d verificações OK\n", (total - t0) - (falhas - f0), total - t0);

    printf("\nRESULTADO: %d/%d verificações passaram%s\n", total - falhas, total,
           falhas ? "  <<< HÁ FALHAS" : "");
    free(cres); free(decr); free(alea);
    return falhas ? 1 : 0;
}
