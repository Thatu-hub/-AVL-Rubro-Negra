// main.c - programa com menu (registro de encomendas da Black Friday)
// EDB II (IMD0039) - Thaliton do Nascimento Silva
//
// o usuario escolhe AVL ou rubro-negra e mexe com os IDs das encomendas
//
// ./amazon_arvores              normal (cores na rubro-negra)
// ./amazon_arvores --sem-cor    sem cor, so [P] e [V]
// ./amazon_arvores --menu-curto menu numa linha so
// ./amazon_arvores --eco < roteiro.txt   repete a entrada (pra gerar os logs)
#include "plataforma.h"   // tem que vir antes dos outros includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "avl.h"
#include "rubronegra.h"

enum { TIPO_AVL = 1, TIPO_RN = 2 };

static int tipo = 0;
static ArvAVL  *avl = NULL;
static ArvLLRB *rn  = NULL;
static int usarCores = 1;
static int compacto = 0;
static int eco = 0;        // mostra o que foi lido (quando a entrada vem de arquivo)

// ---- leitura da entrada ----
static int leLinha(char *buf, int tam) {
    if (fgets(buf, tam, stdin) == NULL) return 0;
    buf[strcspn(buf, "\r\n")] = '\0';
    if (eco) printf("%s\n", buf);
    return 1;
}

// le um inteiro, retorna 0 se for invalido
static int leInteiro(const char *msg, int *valor) {
    char buf[256], *fim;
    printf("%s", msg);
    fflush(stdout);
    if (!leLinha(buf, sizeof(buf))) return -1;          // fim da entrada
    errno = 0;
    long v = strtol(buf, &fim, 10);
    while (*fim == ' ' || *fim == '\t') fim++;
    if (fim == buf || *fim != '\0' || errno != 0 || v < -2147483647L || v > 2147483647L) {
        printf("  Entrada inválida: digite um número inteiro.\n");
        return 0;
    }
    *valor = (int) v;
    return 1;
}

// ---- chama a funcao da arvore escolhida ----
static const char *nomeTipo(void) {
    return tipo == TIPO_AVL ? "Árvore AVL" : "Árvore Rubro-Negra (LLRB)";
}

static void liberaAtual(void) {
    if (avl) { libera_ArvAVL(avl); avl = NULL; }
    if (rn)  { libera_ArvLLRB(rn); rn = NULL; }
}

static void criaAtual(void) {
    if (tipo == TIPO_AVL) { avl = cria_ArvAVL(); zeraEstat_ArvAVL(); }
    else                  { rn = cria_ArvLLRB(); zeraEstat_ArvLLRB(); }
}

// mostra as rotacoes/trocas de cor da ultima operacao
static void relataAjustes(EstatAVL a0, EstatRN r0) {
    if (tipo == TIPO_AVL) {
        EstatAVL a = estat_ArvAVL();
        long ll = a.rotLL - a0.rotLL, rr = a.rotRR - a0.rotRR;
        long lr = a.rotLR - a0.rotLR, rl = a.rotRL - a0.rotRL;
        if (ll + rr + lr + rl == 0) { printf("  Balanceamento: nenhuma rotação necessária.\n"); return; }
        printf("  Balanceamento:");
        if (ll) printf(" %ld x LL (simples à direita)", ll);
        if (rr) printf(" %ld x RR (simples à esquerda)", rr);
        if (lr) printf(" %ld x LR (dupla à direita)", lr);
        if (rl) printf(" %ld x RL (dupla à esquerda)", rl);
        printf("\n");
    } else {
        EstatRN r = estat_ArvLLRB();
        long e = r.rotEsq - r0.rotEsq, d = r.rotDir - r0.rotDir, t = r.trocasCor - r0.trocasCor;
        if (e + d + t == 0) { printf("  Balanceamento: nenhum ajuste necessário.\n"); return; }
        printf("  Balanceamento: %ld rotação(ões) à esquerda, %ld à direita, %ld troca(s) de cor\n", e, d, t);
    }
}

static void inserir(int v) {
    EstatAVL a0 = estat_ArvAVL(); EstatRN r0 = estat_ArvLLRB();
    int ok = (tipo == TIPO_AVL) ? insere_ArvAVL(avl, v) : insere_ArvLLRB(rn, v);
    if (ok) { printf("  Encomenda %d registrada.\n", v); relataAjustes(a0, r0); }
    else      printf("  Encomenda %d JÁ EXISTE: inserção ignorada (IDs são únicos).\n", v);
}

static void remover(int v) {
    EstatAVL a0 = estat_ArvAVL(); EstatRN r0 = estat_ArvLLRB();
    int vazia = (tipo == TIPO_AVL) ? estaVazia_ArvAVL(avl) : estaVazia_ArvLLRB(rn);
    if (vazia) { printf("  A árvore está vazia: nada a remover.\n"); return; }
    int ok = (tipo == TIPO_AVL) ? remove_ArvAVL(avl, v) : remove_ArvLLRB(rn, v);
    if (ok) { printf("  Encomenda %d removida.\n", v); relataAjustes(a0, r0); }
    else      printf("  Encomenda %d NÃO ENCONTRADA: árvore não foi alterada.\n", v);
}

static void buscar(int v) {
    EstatAVL a0 = estat_ArvAVL(); EstatRN r0 = estat_ArvLLRB();
    int ok = (tipo == TIPO_AVL) ? consulta_ArvAVL(avl, v) : consulta_ArvLLRB(rn, v);
    long c = (tipo == TIPO_AVL) ? estat_ArvAVL().comparacoes - a0.comparacoes
                                : estat_ArvLLRB().comparacoes - r0.comparacoes;
    if (ok) printf("  Encomenda %d ENCONTRADA (%ld nó(s) visitado(s)).\n", v, c);
    else    printf("  Encomenda %d NÃO encontrada (%ld nó(s) visitado(s)).\n", v, c);
}

static void verificar(void) {
    char msg[256];
    int ok = (tipo == TIPO_AVL) ? verifica_ArvAVL(avl, msg, sizeof msg)
                                : verifica_ArvLLRB(rn, msg, sizeof msg);
    printf("  [%s] %s\n", ok ? "VÁLIDA" : "INVÁLIDA", msg);
}

static void estatisticas(void) {
    if (tipo == TIPO_AVL) {
        EstatAVL a = estat_ArvAVL();
        printf("  Nós: %d | Altura: %d\n", totalNO_ArvAVL(avl), altura_ArvAVL(avl));
        printf("  Rotações acumuladas: LL=%ld RR=%ld LR=%ld RL=%ld (total em rotações simples: %ld)\n",
               a.rotLL, a.rotRR, a.rotLR, a.rotRL, totalRotacoes_ArvAVL());
    } else {
        EstatRN r = estat_ArvLLRB();
        printf("  Nós: %d | Altura: %d | Altura negra: %d\n",
               totalNO_ArvLLRB(rn), altura_ArvLLRB(rn), alturaNegra_ArvLLRB(rn));
        printf("  Ajustes acumulados: rot. esquerda=%ld rot. direita=%ld trocas de cor=%ld\n",
               r.rotEsq, r.rotDir, r.trocasCor);
    }
}

// troca de arvore mantendo os mesmos IDs
static void trocarEstrutura(void) {
    int n = (tipo == TIPO_AVL) ? totalNO_ArvAVL(avl) : totalNO_ArvLLRB(rn);
    int *v = (int *) malloc((n > 0 ? n : 1) * sizeof(int));
    if (v == NULL) { printf("  Sem memória.\n"); return; }
    n = (tipo == TIPO_AVL) ? emOrdemVetor_ArvAVL(avl, v, n) : emOrdemVetor_ArvLLRB(rn, v, n);
    liberaAtual();
    tipo = (tipo == TIPO_AVL) ? TIPO_RN : TIPO_AVL;
    criaAtual();
    for (int i = 0; i < n; i++) {
        if (tipo == TIPO_AVL) insere_ArvAVL(avl, v[i]); else insere_ArvLLRB(rn, v[i]);
    }
    free(v);
    printf("  Agora usando: %s (%d IDs copiados em ordem crescente).\n", nomeTipo(), n);
}

// ---- menus ----
static int escolheTipo(void) {
    int op;
    for (;;) {
        printf("\nEscolha a estrutura de dados:\n");
        printf("  1 - Árvore AVL\n");
        printf("  2 - Árvore Rubro-Negra (caída para a esquerda - LLRB)\n");
        printf("  0 - Sair\n");
        int r = leInteiro("Opção: ", &op);
        if (r < 0) return 0;
        if (r == 0) continue;
        if (op == 0) return 0;
        if (op == TIPO_AVL || op == TIPO_RN) { tipo = op; criaAtual(); return 1; }
        printf("  Opção inválida.\n");
    }
}

static void menu(void) {
    char buf[4096];
    int op, v;
    for (;;) {
        if (compacto) {
            printf("\n[%s] 1 Inserir  2 Remover  3 Buscar  4 Em ordem  5 Ver  6 Verificar  0 Sair\n",
                   tipo == TIPO_AVL ? "AVL" : "Rubro-Negra");
        } else {
        printf("\n===== %s =====\n", nomeTipo());
        printf("  1 - Inserir encomenda(s)\n");
        printf("  2 - Remover encomenda\n");
        printf("  3 - Buscar encomenda\n");
        printf("  4 - Listar em ordem (crescente)\n");
        printf("  5 - Visualizar estrutura da árvore\n");
        printf("  6 - Verificar propriedades de balanceamento\n");
        printf("  7 - Estatísticas (altura, nós, rotações)\n");
        printf("  8 - Carregar exemplo de demonstração\n");
        printf("  9 - Limpar árvore\n");
        printf(" 10 - Trocar de estrutura (mantendo os IDs)\n");
        printf("  0 - Sair\n");
        }
        int r = leInteiro("Opção: ", &op);
        if (r < 0) return;
        if (r == 0) continue;

        switch (op) {
        case 1:
            printf("IDs separados por espaço: ");
            fflush(stdout);
            if (!leLinha(buf, sizeof buf)) return;
            {
                char *tok = strtok(buf, " \t,;"), *fim;
                int lidos = 0;
                while (tok != NULL) {
                    errno = 0;
                    long x = strtol(tok, &fim, 10);
                    if (*fim != '\0' || errno != 0 || x < -2147483647L || x > 2147483647L)
                        printf("  '%s' ignorado: não é um inteiro válido.\n", tok);
                    else { inserir((int) x); lidos++; }
                    tok = strtok(NULL, " \t,;");
                }
                if (lidos == 0) printf("  Nenhum ID válido informado.\n");
            }
            break;
        case 2:
            if ((r = leInteiro("ID a remover: ", &v)) < 0) return;
            if (r) remover(v);
            break;
        case 3:
            if ((r = leInteiro("ID a buscar: ", &v)) < 0) return;
            if (r) buscar(v);
            break;
        case 4:
            printf("  ");
            if (tipo == TIPO_AVL) emOrdem_ArvAVL(avl); else emOrdem_ArvLLRB(rn);
            break;
        case 5:
            if (tipo == TIPO_AVL) imprime_ArvAVL(avl); else imprime_ArvLLRB(rn, usarCores);
            break;
        case 6: verificar(); break;
        case 7: estatisticas(); break;
        case 8: {
            // exemplos das aulas: {1,2,3,10,4,5,9,7,8,6} na AVL e 10 20 30 25 5 1 2 na LLRB
            static const int exAVL[] = {1, 2, 3, 10, 4, 5, 9, 7, 8, 6};
            static const int exRN[]  = {10, 20, 30, 25, 5, 1, 2};
            const int *ex = (tipo == TIPO_AVL) ? exAVL : exRN;
            int n = (tipo == TIPO_AVL) ? 10 : 7;
            printf("  Inserindo a sequência do slide:");
            for (int i = 0; i < n; i++) printf(" %d", ex[i]);
            printf("\n");
            for (int i = 0; i < n; i++) inserir(ex[i]);
            break;
        }
        case 9:
            liberaAtual(); criaAtual();
            printf("  Árvore esvaziada.\n");
            break;
        case 10: trocarEstrutura(); break;
        case 0: return;
        default: printf("  Opção inválida.\n");
        }
    }
}

int main(int argc, char **argv) {
    prepara_console();
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--sem-cor") == 0) usarCores = 0;
        else if (strcmp(argv[i], "--eco") == 0) eco = 1;
        else if (strcmp(argv[i], "--menu-curto") == 0) compacto = 1;

    printf("==========================================================\n");
    printf(" Amazon Logistics - Black Friday\n");
    printf(" Registro de encomendas com árvores balanceadas\n");
    printf(" EDB II (IMD0039) - Thaliton do Nascimento Silva\n");
    printf("==========================================================\n");

    if (escolheTipo()) menu();
    liberaAtual();
    printf("Encerrando.\n");
    return 0;
}
