// benchmark.c - compara AVL, rubro-negra (LLRB) e ABB comum
// EDB II (IMD0039) - Thaliton do Nascimento Silva
//
// as duas arvores recebem exatamente os mesmos dados (gerados uma vez, com semente fixa)
//
// experimento 1: n = 1000, 10000 e 100000, em ordem crescente, decrescente e
// aleatoria. insere n IDs, busca n que existem + n que nao existem e remove n/2
// experimento 2: 100 mil encomendas ja cadastradas + 1 milhao de operacoes
//
// mostra as tabelas e grava os CSV em resultados/
// ./benchmark [repeticoes]   (padrao 3)
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "avl.h"
#include "rubronegra.h"
#include "abb.h"
#include "plataforma.h"

static int REPS = 3;

static double agora(void) {
    return segundos_agora() * 1e3;
}

static unsigned long long semente;
static unsigned aleat(unsigned n) {
    semente = semente * 6364136223846793005ULL + 1442695040888963407ULL;
    return (unsigned) ((semente >> 33) % n);
}
static void embaralha(int *v, int n) {
    for (int i = n - 1; i > 0; i--) { int j = (int) aleat(i + 1); int t = v[i]; v[i] = v[j]; v[j] = t; }
}

typedef struct {
    double tIns, tBus, tRem;           // tempo em ms (menor das repeticoes)
    int    hIns, hRem;                 // altura depois das insercoes / depois das remocoes
    long   rotIns, rotRem;             // rotacoes (dupla conta como 2)
    long   rotDuplasIns;               // AVL: rotacoes duplas na insercao
    long   coresIns, coresRem;         // LLRB: trocas de cor
    double cmpMedia;                   // nos visitados por busca
} Medida;

static Medida rodaAVL(const int *ins, int n, const int *naoExist, const int *rem, int nr) {
    Medida m; memset(&m, 0, sizeof m);
    m.tIns = m.tBus = m.tRem = 1e18;
    for (int r = 0; r < REPS; r++) {
        ArvAVL *t = cria_ArvAVL();
        zeraEstat_ArvAVL();
        double t0 = agora();
        for (int i = 0; i < n; i++) insere_ArvAVL(t, ins[i]);
        double t1 = agora();
        EstatAVL e = estat_ArvAVL();
        m.hIns = altura_ArvAVL(t);
        m.rotIns = totalRotacoes_ArvAVL();
        m.rotDuplasIns = e.rotLR + e.rotRL;
        long c0 = e.comparacoes;
        double t2 = agora();
        for (int i = 0; i < n; i++) consulta_ArvAVL(t, ins[i]);
        for (int i = 0; i < n; i++) consulta_ArvAVL(t, naoExist[i]);
        double t3 = agora();
        m.cmpMedia = (double) (estat_ArvAVL().comparacoes - c0) / (2.0 * n);
        long rot0 = totalRotacoes_ArvAVL();
        double t4 = agora();
        for (int i = 0; i < nr; i++) remove_ArvAVL(t, rem[i]);
        double t5 = agora();
        m.rotRem = totalRotacoes_ArvAVL() - rot0;
        m.hRem = altura_ArvAVL(t);
        if (t1 - t0 < m.tIns) m.tIns = t1 - t0;
        if (t3 - t2 < m.tBus) m.tBus = t3 - t2;
        if (t5 - t4 < m.tRem) m.tRem = t5 - t4;
        libera_ArvAVL(t);
    }
    return m;
}

static Medida rodaRN(const int *ins, int n, const int *naoExist, const int *rem, int nr) {
    Medida m; memset(&m, 0, sizeof m);
    m.tIns = m.tBus = m.tRem = 1e18;
    for (int r = 0; r < REPS; r++) {
        ArvLLRB *t = cria_ArvLLRB();
        zeraEstat_ArvLLRB();
        double t0 = agora();
        for (int i = 0; i < n; i++) insere_ArvLLRB(t, ins[i]);
        double t1 = agora();
        EstatRN e = estat_ArvLLRB();
        m.hIns = altura_ArvLLRB(t);
        m.rotIns = e.rotEsq + e.rotDir;
        m.coresIns = e.trocasCor;
        double t2 = agora();
        for (int i = 0; i < n; i++) consulta_ArvLLRB(t, ins[i]);
        for (int i = 0; i < n; i++) consulta_ArvLLRB(t, naoExist[i]);
        double t3 = agora();
        EstatRN e2 = estat_ArvLLRB();
        m.cmpMedia = (double) (e2.comparacoes - e.comparacoes) / (2.0 * n);
        double t4 = agora();
        for (int i = 0; i < nr; i++) remove_ArvLLRB(t, rem[i]);
        double t5 = agora();
        EstatRN e3 = estat_ArvLLRB();
        m.rotRem = (e3.rotEsq + e3.rotDir) - (e2.rotEsq + e2.rotDir);
        m.coresRem = e3.trocasCor - e2.trocasCor;
        m.hRem = altura_ArvLLRB(t);
        if (t1 - t0 < m.tIns) m.tIns = t1 - t0;
        if (t3 - t2 < m.tBus) m.tBus = t3 - t2;
        if (t5 - t4 < m.tRem) m.tRem = t5 - t4;
        libera_ArvLLRB(t);
    }
    return m;
}

// ABB sem balanceamento: so insere e busca, pra mostrar o problema
static void rodaABB(const int *ins, int n, const int *naoExist, int *altura, double *cmpMedia, double *tIns) {
    ArvABB *t = cria_ArvABB();
    double t0 = agora();
    for (int i = 0; i < n; i++) insere_ArvABB(t, ins[i]);
    *tIns = agora() - t0;
    *altura = altura_ArvABB(t);
    long c = 0;
    for (int i = 0; i < n; i++) consulta_ArvABB(t, ins[i], &c);
    for (int i = 0; i < n; i++) consulta_ArvABB(t, naoExist[i], &c);
    *cmpMedia = (double) c / (2.0 * n);
    libera_ArvABB(t);
}

// ---- experimento 2: Black Friday ----
typedef struct { int tipo; int valor; } Op;   // 0 insere, 1 busca, 2 remove

// gera as operacoes uma vez so, as duas arvores fazem a mesma coisa
static Op *geraCarga(int n0, int nops, int pIns, int pBus, int *proxID) {
    Op *ops = (Op *) malloc(nops * sizeof(Op));
    int *vivos = (int *) malloc((n0 + nops) * sizeof(int));
    int nv = 0, id = 1;
    for (int i = 0; i < n0; i++) vivos[nv++] = id++;     // IDs em sequencia
    for (int i = 0; i < nops; i++) {
        int p = (int) aleat(100);
        if (p < pIns || nv == 0) { ops[i].tipo = 0; ops[i].valor = id; vivos[nv++] = id++; }
        else if (p < pIns + pBus) { ops[i].tipo = 1; ops[i].valor = vivos[aleat(nv)]; }
        else {                                            // entrega: remove um ID que existe
            int k = (int) aleat(nv);
            ops[i].tipo = 2; ops[i].valor = vivos[k];
            vivos[k] = vivos[--nv];
        }
    }
    *proxID = id;
    free(vivos);
    return ops;
}

typedef struct { double ms; int altura; long rot; long cores; double cmp; } MedidaMix;

static MedidaMix mixAVL(int n0, const Op *ops, int nops) {
    MedidaMix m = {1e18, 0, 0, 0, 0};
    for (int r = 0; r < REPS; r++) {
        ArvAVL *t = cria_ArvAVL();
        for (int i = 1; i <= n0; i++) insere_ArvAVL(t, i);
        zeraEstat_ArvAVL();
        long buscas = 0;
        double t0 = agora();
        for (int i = 0; i < nops; i++) {
            if (ops[i].tipo == 0) insere_ArvAVL(t, ops[i].valor);
            else if (ops[i].tipo == 1) { consulta_ArvAVL(t, ops[i].valor); buscas++; }
            else remove_ArvAVL(t, ops[i].valor);
        }
        double dt = agora() - t0;
        if (dt < m.ms) m.ms = dt;
        m.altura = altura_ArvAVL(t);
        m.rot = totalRotacoes_ArvAVL();
        m.cmp = buscas ? (double) estat_ArvAVL().comparacoes / buscas : 0;
        libera_ArvAVL(t);
    }
    return m;
}

static MedidaMix mixRN(int n0, const Op *ops, int nops) {
    MedidaMix m = {1e18, 0, 0, 0, 0};
    for (int r = 0; r < REPS; r++) {
        ArvLLRB *t = cria_ArvLLRB();
        for (int i = 1; i <= n0; i++) insere_ArvLLRB(t, i);
        zeraEstat_ArvLLRB();
        long buscas = 0;
        double t0 = agora();
        for (int i = 0; i < nops; i++) {
            if (ops[i].tipo == 0) insere_ArvLLRB(t, ops[i].valor);
            else if (ops[i].tipo == 1) { consulta_ArvLLRB(t, ops[i].valor); buscas++; }
            else remove_ArvLLRB(t, ops[i].valor);
        }
        double dt = agora() - t0;
        if (dt < m.ms) m.ms = dt;
        EstatRN e = estat_ArvLLRB();
        m.altura = altura_ArvLLRB(t);
        m.rot = e.rotEsq + e.rotDir;
        m.cores = e.trocasCor;
        // o remove da LLRB faz uma busca antes que nao entra na contagem
        m.cmp = buscas ? (double) e.comparacoes / buscas : 0;
        libera_ArvLLRB(t);
    }
    return m;
}

int main(int argc, char **argv) {
    prepara_console();
    if (argc > 1) REPS = atoi(argv[1]) > 0 ? atoi(argv[1]) : 3;
    const int tamanhos[] = {1000, 10000, 100000};
    const char *ordens[] = {"crescente", "decrescente", "aleatoria"};

    FILE *csv = fopen("resultados/experimento1.csv", "w");
    if (csv) fprintf(csv, "n,ordem,arvore,altura_insercao,rotacoes_insercao,rot_duplas_insercao,"
                          "trocas_cor_insercao,comparacoes_media_busca,altura_apos_remocao,"
                          "rotacoes_remocao,trocas_cor_remocao,ms_insercao,ms_busca,ms_remocao\n");

    printf("Experimento 1 - mesmos dados nas duas árvores (menor tempo de %d repetições)\n", REPS);
    printf("Altura: arestas da raiz à folha mais funda (folha = 0). Rotações da AVL contadas como\n");
    printf("rotações simples (uma dupla = 2). Cmp = nós visitados por busca (média).\n\n");
    printf("%-7s %-12s %-6s | %5s %8s %8s %6s | %5s %8s %8s | %8s %8s %8s\n",
           "n", "ordem", "árvore", "h", "rot.ins", "cores", "cmp", "h.rem", "rot.rem", "cores.r",
           "ms.ins", "ms.bus", "ms.rem");

    for (int a = 0; a < 3; a++) {
        int n = tamanhos[a];
        int *ins = (int *) malloc(n * sizeof(int)), *nao = (int *) malloc(n * sizeof(int)),
            *rem = (int *) malloc(n * sizeof(int));
        for (int o = 0; o < 3; o++) {
            // IDs pares (2..2n), os impares servem como IDs que nao existem
            for (int i = 0; i < n; i++) ins[i] = 2 * (i + 1);
            if (o == 1) for (int i = 0; i < n; i++) ins[i] = 2 * (n - i);
            semente = 12345 + n;
            if (o == 2) embaralha(ins, n);
            for (int i = 0; i < n; i++) nao[i] = 2 * i + 1;
            semente = 777 + n;
            embaralha(nao, n);
            for (int i = 0; i < n; i++) rem[i] = 2 * (i + 1);
            semente = 4242 + n;
            embaralha(rem, n);                 // remove metade, em ordem aleatoria
            int nr = n / 2;

            Medida mA = rodaAVL(ins, n, nao, rem, nr);
            Medida mR = rodaRN(ins, n, nao, rem, nr);
            printf("%-7d %-12s %-6s | %5d %8ld %8s %6.2f | %5d %8ld %8s | %8.2f %8.2f %8.2f\n",
                   n, ordens[o], "AVL", mA.hIns, mA.rotIns, "-", mA.cmpMedia, mA.hRem, mA.rotRem, "-",
                   mA.tIns, mA.tBus, mA.tRem);
            printf("%-7s %-12s %-6s | %5d %8ld %8ld %6.2f | %5d %8ld %8ld | %8.2f %8.2f %8.2f\n",
                   "", "", "LLRB", mR.hIns, mR.rotIns, mR.coresIns, mR.cmpMedia, mR.hRem, mR.rotRem,
                   mR.coresRem, mR.tIns, mR.tBus, mR.tRem);
            if (n <= 10000) {
                int hB; double cB, tB;
                rodaABB(ins, n, nao, &hB, &cB, &tB);
                printf("%-7s %-12s %-6s | %5d %8s %8s %6.1f | %5s %8s %8s | %8.2f %8s %8s\n",
                       "", "", "ABB", hB, "-", "-", cB, "-", "-", "-", tB, "-", "-");
                if (csv) fprintf(csv, "%d,%s,ABB,%d,,,,%.2f,,,,%.3f,,\n", n, ordens[o], hB, cB, tB);
            }
            if (csv) {
                fprintf(csv, "%d,%s,AVL,%d,%ld,%ld,,%.2f,%d,%ld,,%.3f,%.3f,%.3f\n", n, ordens[o],
                        mA.hIns, mA.rotIns, mA.rotDuplasIns, mA.cmpMedia, mA.hRem, mA.rotRem,
                        mA.tIns, mA.tBus, mA.tRem);
                fprintf(csv, "%d,%s,LLRB,%d,%ld,,%ld,%.2f,%d,%ld,%ld,%.3f,%.3f,%.3f\n", n, ordens[o],
                        mR.hIns, mR.rotIns, mR.coresIns, mR.cmpMedia, mR.hRem, mR.rotRem, mR.coresRem,
                        mR.tIns, mR.tBus, mR.tRem);
            }
        }
        printf("\n");
        free(ins); free(nao); free(rem);
    }
    if (csv) fclose(csv);

    // ---- experimento 2 ----
    const int n0 = 100000, nops = 1000000;
    struct { const char *nome; int pIns, pBus; } perfis[] = {
        {"pico de pedidos (50% ins, 30% busca, 20% rem)", 50, 30},
        {"rastreamento   (10% ins, 85% busca,  5% rem)", 10, 85},
    };
    FILE *csv2 = fopen("resultados/experimento2.csv", "w");
    if (csv2) fprintf(csv2, "perfil,arvore,ms_total,altura_final,rotacoes,trocas_cor,comparacoes_media_busca\n");
    printf("Experimento 2 - Black Friday: %d encomendas pré-cadastradas (IDs sequenciais) + %d operações\n",
           n0, nops);
    printf("Novos pedidos recebem IDs crescentes; entregas removem IDs aleatórios existentes.\n\n");
    printf("%-48s %-5s | %9s %6s %9s %9s %6s\n", "perfil", "árv.", "ms", "h", "rotações", "cores", "cmp");
    for (int p = 0; p < 2; p++) {
        int prox;
        semente = 2026 + p;
        Op *ops = geraCarga(n0, nops, perfis[p].pIns, perfis[p].pBus, &prox);
        MedidaMix a = mixAVL(n0, ops, nops);
        MedidaMix r = mixRN(n0, ops, nops);
        printf("%-48s %-5s | %9.1f %6d %9ld %9s %6.2f\n", perfis[p].nome, "AVL", a.ms, a.altura, a.rot, "-", a.cmp);
        printf("%-48s %-5s | %9.1f %6d %9ld %9ld %6.2f\n", "", "LLRB", r.ms, r.altura, r.rot, r.cores, r.cmp);
        if (csv2) {
            fprintf(csv2, "\"%s\",AVL,%.2f,%d,%ld,,%.2f\n", perfis[p].nome, a.ms, a.altura, a.rot, a.cmp);
            fprintf(csv2, "\"%s\",LLRB,%.2f,%d,%ld,%ld,%.2f\n", perfis[p].nome, r.ms, r.altura, r.rot, r.cores, r.cmp);
        }
        free(ops);
    }
    if (csv2) fclose(csv2);
    printf("\nCSV gravados em resultados/ (se a pasta existir).\n");
    return 0;
}
