// simulacao.c - simulacao da Black Friday com muitos pedidos
// EDB II (IMD0039) - Thaliton do Nascimento Silva
//
// o que eu considerei (os numeros exatos sao suposicoes minhas):
//   - 11 dias, de 22/11 a 02/12, como a Black Friday 2024 da Amazon Brasil
//   - N pedidos com IDs em sequencia
//   - mais pedidos na Black Friday (29/11) e na Cyber Monday (02/12), e mais a noite
//   - cada pedido e consultado 3 vezes antes de ser entregue
//   - entrega entre 24h e 96h depois, ai o ID sai da arvore
//   - 2% das consultas sao de IDs que nao existem
//
// as operacoes sao geradas uma vez (semente fixa) e rodam na AVL e depois na LLRB.
// tambem mede a ABB comum com poucos pedidos pra estimar quanto ela levaria
//
// ./simulacao             serie de 10 mil a 200 mil pedidos + projecao
// ./simulacao 10000000    rodada grande (uns 1 min e 1 GB de RAM)
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "avl.h"
#include "rubronegra.h"
#include "abb.h"
#include "plataforma.h"

#define DIAS  11
#define HORAS (DIAS * 24)

static const char *NOME_DIA[DIAS] = {"22/11", "23/11", "24/11", "25/11", "26/11", "27/11",
                                     "28/11", "29/11 BF", "30/11", "01/12", "02/12 CM"};
// peso de cada dia: pico na Black Friday e na Cyber Monday
static const double PESO_DIA[DIAS] = {1.0, 1.1, 1.0, 1.0, 1.1, 1.3, 1.6, 3.5, 2.2, 1.8, 2.8};
// peso de cada hora: madrugada fraca, noite forte
static const double PESO_HORA[24] = {0.5, 0.3, 0.2, 0.2, 0.2, 0.3, 0.5, 0.8, 1.0, 1.1, 1.2, 1.2,
                                     1.3, 1.2, 1.1, 1.1, 1.2, 1.3, 1.5, 1.7, 1.9, 1.9, 1.5, 0.9};

static unsigned long long semente = 20241122ULL;
static unsigned aleat(unsigned n) {
    semente = semente * 6364136223846793005ULL + 1442695040888963407ULL;
    return (unsigned) ((semente >> 33) % n);
}

static double agora(void) {
    return segundos_agora();
}

enum { OP_INS = 0, OP_BUS = 1, OP_REM = 2 };

typedef struct {
    int *ins_ini, *ins_qtd;      // por hora: primeiro ID e quantos pedidos
    int *bus, *bus_ini, *bus_qtd;// buscas de cada hora
    int *rem, *rem_ini, *rem_qtd;// remocoes de cada hora
    long nIns, nBus, nRem, nBusInex;
} Carga;

static Carga geraCarga(int N) {
    Carga c; memset(&c, 0, sizeof c);
    c.ins_ini = calloc(HORAS, sizeof(int)); c.ins_qtd = calloc(HORAS, sizeof(int));
    c.bus_ini = calloc(HORAS + 1, sizeof(int)); c.bus_qtd = calloc(HORAS, sizeof(int));
    c.rem_ini = calloc(HORAS + 1, sizeof(int)); c.rem_qtd = calloc(HORAS, sizeof(int));

    // 1) espalha os N pedidos pelas horas
    double tot = 0;
    for (int h = 0; h < HORAS; h++) tot += PESO_DIA[h / 24] * PESO_HORA[h % 24];
    int id = 1, acumulado = 0;
    double acc = 0;
    for (int h = 0; h < HORAS; h++) {
        acc += PESO_DIA[h / 24] * PESO_HORA[h % 24];
        int alvo = (int) (acc / tot * N + 0.5);
        c.ins_ini[h] = id;
        c.ins_qtd[h] = alvo - acumulado;
        id += c.ins_qtd[h];
        acumulado = alvo;
    }
    c.nIns = N;

    // 2) sorteia a hora da entrega e das 3 consultas de cada pedido
    int *hEnt = malloc((size_t) N * sizeof(int));
    int (*hBus)[3] = malloc((size_t) N * sizeof(*hBus));
    for (int h = 0; h < HORAS; h++) {
        for (int k = 0; k < c.ins_qtd[h]; k++) {
            int i = c.ins_ini[h] + k - 1;
            int e = h + 24 + (int) aleat(73);          // entrega entre 24 e 96h
            hEnt[i] = e;
            int fim = (e < HORAS) ? e : HORAS;          // consulta antes da entrega
            for (int j = 0; j < 3; j++) hBus[i][j] = h + (int) aleat((unsigned) (fim - h));
            if (e < HORAS) c.rem_qtd[e]++;
            for (int j = 0; j < 3; j++) c.bus_qtd[hBus[i][j]]++;
        }
    }
    // 2% de consultas a IDs que nao existem
    long nInex = (long) (0.02 * 3.0 * N);
    int *hInex = malloc((size_t) nInex * sizeof(int));
    for (long i = 0; i < nInex; i++) { hInex[i] = (int) aleat(HORAS); c.bus_qtd[hInex[i]]++; }

    // 3) agrupa tudo por hora
    for (int h = 0; h < HORAS; h++) {
        c.bus_ini[h + 1] = c.bus_ini[h] + c.bus_qtd[h];
        c.rem_ini[h + 1] = c.rem_ini[h] + c.rem_qtd[h];
    }
    c.nBus = c.bus_ini[HORAS]; c.nRem = c.rem_ini[HORAS]; c.nBusInex = nInex;
    c.bus = malloc((size_t) c.nBus * sizeof(int));
    c.rem = malloc((size_t) (c.nRem > 0 ? c.nRem : 1) * sizeof(int));
    int *pb = calloc(HORAS, sizeof(int)), *pr = calloc(HORAS, sizeof(int));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 3; j++) { int h = hBus[i][j]; c.bus[c.bus_ini[h] + pb[h]++] = i + 1; }
        if (hEnt[i] < HORAS) { int h = hEnt[i]; c.rem[c.rem_ini[h] + pr[h]++] = i + 1; }
    }
    for (long i = 0; i < nInex; i++) {
        int h = hInex[i];
        c.bus[c.bus_ini[h] + pb[h]++] = -(int) (1 + aleat(1000000));   // ID que nao existe
    }
    // embaralha dentro da hora (as consultas nao chegam em ordem)
    for (int h = 0; h < HORAS; h++) {
        int *v = c.bus + c.bus_ini[h]; int n = c.bus_qtd[h];
        for (int i = n - 1; i > 0; i--) { int j = (int) aleat(i + 1); int t = v[i]; v[i] = v[j]; v[j] = t; }
        v = c.rem + c.rem_ini[h]; n = c.rem_qtd[h];
        for (int i = n - 1; i > 0; i--) { int j = (int) aleat(i + 1); int t = v[i]; v[i] = v[j]; v[j] = t; }
    }
    free(hEnt); free(hBus); free(hInex); free(pb); free(pr);
    return c;
}

static void liberaCarga(Carga *c) {
    free(c->ins_ini); free(c->ins_qtd); free(c->bus); free(c->bus_ini); free(c->bus_qtd);
    free(c->rem); free(c->rem_ini); free(c->rem_qtd);
}

typedef struct {
    double seg, segIns, segBus, segRem;
    long rotIns, rotRem, coresIns, coresRem;
    double visitas;                 // nos visitados por busca
    int hFimDia[DIAS], ativosFimDia[DIAS], hMax, ativosMax;
    long achadas, naoAchadas;
} Resultado;

// roda a carga em qualquer uma das arvores (por ponteiro de funcao)
typedef struct {
    void *(*cria)(void); void (*libera)(void *);
    int (*ins)(void *, int); int (*rem)(void *, int); int (*bus)(void *, int);
    int (*alt)(void *); long (*rot)(void); long (*cores)(void); long (*cmp)(void); void (*zera)(void);
} Arv;

static Resultado executa(const Arv *A, const Carga *c) {
    Resultado r; memset(&r, 0, sizeof r);
    void *t = A->cria();
    A->zera();
    int ativos = 0;
    for (int h = 0; h < HORAS; h++) {
        long rot0 = A->rot(), cor0 = A->cores();
        double t0 = agora();
        for (int k = 0; k < c->ins_qtd[h]; k++) ativos += A->ins(t, c->ins_ini[h] + k);
        double t1 = agora();
        r.rotIns += A->rot() - rot0; r.coresIns += A->cores() - cor0;
        for (int k = 0; k < c->bus_qtd[h]; k++) {
            if (A->bus(t, c->bus[c->bus_ini[h] + k])) r.achadas++; else r.naoAchadas++;
        }
        double t2 = agora();
        rot0 = A->rot(); cor0 = A->cores();
        for (int k = 0; k < c->rem_qtd[h]; k++) ativos -= A->rem(t, c->rem[c->rem_ini[h] + k]);
        double t3 = agora();
        r.rotRem += A->rot() - rot0; r.coresRem += A->cores() - cor0;
        r.segIns += t1 - t0; r.segBus += t2 - t1; r.segRem += t3 - t2;
        if (ativos > r.ativosMax) r.ativosMax = ativos;
        if (h % 24 == 23) {
            int d = h / 24;
            r.hFimDia[d] = A->alt(t);
            r.ativosFimDia[d] = ativos;
            if (r.hFimDia[d] > r.hMax) r.hMax = r.hFimDia[d];
        }
    }
    r.seg = r.segIns + r.segBus + r.segRem;
    r.visitas = (double) A->cmp() / (double) c->nBus;
    A->libera(t);
    return r;
}

// adaptadores pra cada arvore
static void *cA(void) { return cria_ArvAVL(); }
static void lA(void *t) { libera_ArvAVL((ArvAVL *) t); }
static int iA(void *t, int v) { return insere_ArvAVL((ArvAVL *) t, v); }
static int rA(void *t, int v) { return remove_ArvAVL((ArvAVL *) t, v); }
static int bA(void *t, int v) { return consulta_ArvAVL((ArvAVL *) t, v); }
static int hA(void *t) { return altura_ArvAVL((ArvAVL *) t); }
static long roA(void) { return totalRotacoes_ArvAVL(); }
static long coA(void) { return 0; }
static long cmA(void) { return estat_ArvAVL().comparacoes; }

static void *cR(void) { return cria_ArvLLRB(); }
static void lR(void *t) { libera_ArvLLRB((ArvLLRB *) t); }
static int iR(void *t, int v) { return insere_ArvLLRB((ArvLLRB *) t, v); }
static int rR(void *t, int v) { return remove_ArvLLRB((ArvLLRB *) t, v); }
static int bR(void *t, int v) { return consulta_ArvLLRB((ArvLLRB *) t, v); }
static int hR(void *t) { return altura_ArvLLRB((ArvLLRB *) t); }
static long roR(void) { return totalRotacoes_ArvLLRB(); }
static long coR(void) { return estat_ArvLLRB().trocasCor; }
static long cmR(void) { return estat_ArvLLRB().comparacoes; }

static void imprime(const char *nome, const Resultado *r, const Carga *c, FILE *csv) {
    long ops = c->nIns + c->nBus + c->nRem;
    printf("\n--- %s ---\n", nome);
    printf("Tempo total: %.2f s  (inserção %.2f s | busca %.2f s | remoção %.2f s)\n",
           r->seg, r->segIns, r->segBus, r->segRem);
    printf("Vazão: %.2f milhões de operações/s (1 núcleo)\n", ops / r->seg / 1e6);
    printf("Rotações: inserção %ld (%.2f/pedido) | remoção %ld (%.2f/entrega)\n",
           r->rotIns, (double) r->rotIns / c->nIns, r->rotRem, c->nRem ? (double) r->rotRem / c->nRem : 0.0);
    printf("Trocas de cor: inserção %ld | remoção %ld\n", r->coresIns, r->coresRem);
    printf("Nós visitados por busca: %.2f | buscas encontradas %ld, não encontradas %ld\n",
           r->visitas, r->achadas, r->naoAchadas);
    printf("Pico de pedidos ativos: %d | altura máxima: %d\n", r->ativosMax, r->hMax);
    printf("Fim de cada dia (ativos / altura):");
    for (int d = 0; d < DIAS; d++) printf(" %s=%d/%d", NOME_DIA[d], r->ativosFimDia[d], r->hFimDia[d]);
    printf("\n");
    if (csv) fprintf(csv, "%s,%.3f,%.3f,%.3f,%.3f,%.3f,%ld,%ld,%ld,%ld,%.3f,%d,%d\n", nome, r->seg, r->segIns,
                     r->segBus, r->segRem, ops / r->seg / 1e6, r->rotIns, r->rotRem, r->coresIns, r->coresRem,
                     r->visitas, r->ativosMax, r->hMax);
}

// reta y = a + b*x por minimos quadrados (pra projetar a altura)
static void ajusta(const double *x, const double *y, int n, double *a, double *b) {
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (int i = 0; i < n; i++) { sx += x[i]; sy += y[i]; sxx += x[i] * x[i]; sxy += x[i] * y[i]; }
    *b = (n * sxy - sx * sy) / (n * sxx - sx * sx);
    *a = (sy - *b * sx) / n;
}

static double log2d(double v) { return log(v) / log(2.0); }

// sem argumento: roda a serie pequena (rapido) e projeta pra escala real
static int modoSerie(void) {
    const int ns[] = {10000, 50000, 100000, 200000};
    const int K = 4;
    Arv AVL = {cA, lA, iA, rA, bA, hA, roA, coA, cmA, zeraEstat_ArvAVL};
    Arv RN  = {cR, lR, iR, rR, bR, hR, roR, coR, cmR, zeraEstat_ArvLLRB};
    double x[4], hA_[4], hR_[4], vA[4], vR[4], qA[4], qR[4], razaoPico = 0, razaoEntregas = 0;
    double rotInsA = 0, rotRemA = 0, rotInsR = 0, rotRemR = 0, corR = 0;

    FILE *csv = fopen("resultados/simulacao_serie.csv", "w");
    if (csv) fprintf(csv, "pedidos,arvore,pico_ativos,altura_maxima,visitas_por_busca,rotacoes_por_pedido,"
                          "rotacoes_por_entrega,trocas_cor_por_operacao,seg_total,milhoes_ops_por_s\n");
    printf("Simulação Black Friday em escala reduzida (mesmo calendário de 11 dias, 22/11 a 02/12)\n");
    printf("Cada pedido: 1 inserção, 3 consultas, 1 remoção na entrega (24-96 h); 2%% de consultas inválidas.\n\n");
    printf("%9s %-5s | %10s %7s %8s %9s %9s %9s %8s\n", "pedidos", "árv.", "pico ativ.", "altura",
           "visitas", "rot/ped.", "rot/entr.", "cor/op", "tempo");
    for (int k = 0; k < K; k++) {
        semente = 20241122ULL;
        Carga c = geraCarga(ns[k]);
        long ops = c.nIns + c.nBus + c.nRem;
        Resultado a = executa(&AVL, &c), r = executa(&RN, &c);
        x[k] = log2d(a.ativosMax);
        hA_[k] = a.hMax; hR_[k] = r.hMax; vA[k] = a.visitas; vR[k] = r.visitas;
        razaoPico = (double) a.ativosMax / ns[k];
        rotInsA = (double) a.rotIns / c.nIns; rotRemA = (double) a.rotRem / c.nRem;
        rotInsR = (double) r.rotIns / c.nIns; rotRemR = (double) r.rotRem / c.nRem;
        corR = (double) (r.coresIns + r.coresRem) / ops;
        qA[k] = rotRemA; qR[k] = rotRemR; razaoEntregas = (double) c.nRem / ns[k];
        printf("%9d %-5s | %10d %7d %8.2f %9.2f %9.2f %9s %7.2fs\n", ns[k], "AVL", a.ativosMax, a.hMax,
               a.visitas, rotInsA, rotRemA, "0", a.seg);
        printf("%9s %-5s | %10s %7d %8.2f %9.2f %9.2f %9.2f %7.2fs\n", "", "LLRB", "", r.hMax,
               r.visitas, rotInsR, rotRemR, corR, r.seg);
        if (csv) {
            fprintf(csv, "%d,AVL,%d,%d,%.3f,%.3f,%.3f,0,%.3f,%.3f\n", ns[k], a.ativosMax, a.hMax, a.visitas,
                    rotInsA, rotRemA, a.seg, ops / a.seg / 1e6);
            fprintf(csv, "%d,LLRB,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n", ns[k], r.ativosMax, r.hMax, r.visitas,
                    rotInsR, rotRemR, corR, r.seg, ops / r.seg / 1e6);
        }
        liberaCarga(&c);
    }
    if (csv) fclose(csv);

    // a altura e as visitas crescem com log2 dos pedidos ativos
    double aA, bA, aR, bR, avA, bvA, avR, bvR, aqA, bqA, aqR, bqR;
    ajusta(x, hA_, K, &aA, &bA); ajusta(x, hR_, K, &aR, &bR);
    ajusta(x, vA, K, &avA, &bvA); ajusta(x, vR, K, &avR, &bvR);
    ajusta(x, qA, K, &aqA, &bqA); ajusta(x, qR, K, &aqR, &bqR);
    printf("\nProjeção para a escala real (altura, visitas e rotações por entrega ~ a + b*log2(ativos))\n");
    printf("%14s %12s | %9s %9s | %9s %9s | %22s\n", "pedidos", "pico ativos", "h AVL", "h LLRB",
           "vis. AVL", "vis. LLRB", "rotações por entrega");
    const double alvos[] = {1e6, 10e6, 130e6};
    for (int i = 0; i < 3; i++) {
        double pico = razaoPico * alvos[i], lx = log2d(pico);
        (void) razaoEntregas;
        printf("%14.0f %12.0f | %9.1f %9.1f | %9.1f %9.1f | AVL %.2f  LLRB %.1f\n", alvos[i], pico,
               aA + bA * lx, aR + bR * lx, avA + bvA * lx, avR + bvR * lx, aqA + bqA * lx, aqR + bqR * lx);
    }
    printf("(130 milhões = produtos ofertados pela Amazon Brasil na Black Friday 2024, IstoÉ Dinheiro)\n");

    // ABB comum: mede com poucos IDs e projeta (cresce com n ao quadrado)
    printf("\nABB sem balanceamento com IDs sequenciais:\n");
    int nb[] = {10000, 20000, 40000};
    double ult = 0;
    for (int k = 0; k < 3; k++) {
        ArvABB *t = cria_ArvABB();
        double a0 = agora();
        for (int i = 1; i <= nb[k]; i++) insere_ArvABB(t, i);
        ult = agora() - a0;
        printf("  %6d IDs: %.3f s, altura %d\n", nb[k], ult, altura_ArvABB(t));
        libera_ArvABB(t);
    }
    for (int i = 0; i < 2; i++) {
        double p = ult * (alvos[i] / 40000.0) * (alvos[i] / 40000.0);
        printf("  projeção para %.0f pedidos: %.0f s (%.1f horas) só para cadastrar\n", alvos[i], p, p / 3600);
    }
    printf("\nPara conferir a projeção em escala real: ./simulacao 10000000  (cerca de 1 min e 1 GB de RAM)\n");
    return 0;
}

int main(int argc, char **argv) {
    prepara_console();
    if (argc < 2) return modoSerie();
    int N = atoi(argv[1]);
    if (N < 1000) N = 1000;
    printf("Simulação Black Friday: %d pedidos em %d dias (22/11 a 02/12)\n", N, DIAS);
    double t0 = agora();
    Carga c = geraCarga(N);
    printf("Carga gerada em %.1f s: %ld inserções, %ld buscas (%ld de IDs inexistentes), %ld remoções\n",
           agora() - t0, c.nIns, c.nBus, c.nBusInex, c.nRem);
    printf("Pedidos por dia:");
    for (int d = 0; d < DIAS; d++) {
        long s = 0;
        for (int h = d * 24; h < d * 24 + 24; h++) s += c.ins_qtd[h];
        printf(" %s=%ld", NOME_DIA[d], s);
    }
    int hPico = 0;
    for (int h = 0; h < HORAS; h++) if (c.ins_qtd[h] > c.ins_qtd[hPico]) hPico = h;
    long opsPico = c.ins_qtd[hPico] + c.bus_qtd[hPico] + c.rem_qtd[hPico];
    printf("\nHora de pico: %s às %02dh, %d pedidos novos e %ld operações no total\n",
           NOME_DIA[hPico / 24], hPico % 24, c.ins_qtd[hPico], opsPico);

    FILE *csv = fopen("resultados/simulacao_blackfriday.csv", "w");
    if (csv) fprintf(csv, "arvore,seg_total,seg_insercao,seg_busca,seg_remocao,milhoes_ops_por_s,"
                          "rotacoes_insercao,rotacoes_remocao,trocas_cor_insercao,trocas_cor_remocao,"
                          "visitas_por_busca,pico_ativos,altura_maxima\n");

    Arv AVL = {cA, lA, iA, rA, bA, hA, roA, coA, cmA, zeraEstat_ArvAVL};
    Arv RN  = {cR, lR, iR, rR, bR, hR, roR, coR, cmR, zeraEstat_ArvLLRB};
    Resultado ra = executa(&AVL, &c);
    imprime("AVL", &ra, &c, csv);
    Resultado rr = executa(&RN, &c);
    imprime("LLRB", &rr, &c, csv);
    if (csv) fclose(csv);

    liberaCarga(&c);
    return 0;
}
