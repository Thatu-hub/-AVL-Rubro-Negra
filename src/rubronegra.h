// rubronegra.h - arvore rubro-negra caida para a esquerda (LLRB)
// EDB II (IMD0039) - Thaliton do Nascimento Silva
//
// versao LLRB (Sedgewick). a struct e NO_RN pra nao bater com a da AVL
#ifndef RUBRONEGRA_H
#define RUBRONEGRA_H

#define RED   1
#define BLACK 0

typedef struct NO_RN *ArvLLRB;

typedef struct {
    long rotEsq;       // rotacoes a esquerda
    long rotDir;       // rotacoes a direita
    long trocasCor;    // trocas de cor
    long comparacoes;  // nos visitados nas buscas
} EstatRN;

ArvLLRB *cria_ArvLLRB(void);
void     libera_ArvLLRB(ArvLLRB *raiz);

// retorna 1 se deu certo, 0 se nao (repetido, nao existe ou arvore vazia)
int insere_ArvLLRB(ArvLLRB *raiz, int valor);
int remove_ArvLLRB(ArvLLRB *raiz, int valor);
int consulta_ArvLLRB(ArvLLRB *raiz, int valor);

int  altura_ArvLLRB(ArvLLRB *raiz);        // vazia = -1
int  alturaNegra_ArvLLRB(ArvLLRB *raiz);   // qtd de nos pretos ate o NULL
int  totalNO_ArvLLRB(ArvLLRB *raiz);
int  estaVazia_ArvLLRB(ArvLLRB *raiz);

void emOrdem_ArvLLRB(ArvLLRB *raiz);
int  emOrdemVetor_ArvLLRB(ArvLLRB *raiz, int *v, int max);
int  preOrdemVetor_ArvLLRB(ArvLLRB *raiz, int *v, int *cores, int max);
void imprime_ArvLLRB(ArvLLRB *raiz, int usarCores);   // 1 = mostra com cor no terminal

// confere as regras da rubro-negra, a regra da LLRB e a ordem
int verifica_ArvLLRB(ArvLLRB *raiz, char *msg, int tam);

EstatRN estat_ArvLLRB(void);
void    zeraEstat_ArvLLRB(void);
long    totalRotacoes_ArvLLRB(void);

#endif
