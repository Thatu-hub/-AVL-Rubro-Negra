// avl.h - arvore AVL
// EDB II (IMD0039) - Thaliton do Nascimento Silva
//
// mesmo esquema do TAD visto em aula: ArvAVL e um ponteiro pra raiz.
// a struct se chama NO_AVL pra nao dar conflito com a da rubro-negra
#ifndef AVL_H
#define AVL_H

typedef struct NO_AVL *ArvAVL;

// contadores pra analise (rotacoes e comparacoes)
typedef struct {
    long rotLL;        // simples a direita
    long rotRR;        // simples a esquerda
    long rotLR;        // dupla a direita
    long rotRL;        // dupla a esquerda
    long comparacoes;  // nos visitados nas buscas
} EstatAVL;

ArvAVL *cria_ArvAVL(void);
void    libera_ArvAVL(ArvAVL *raiz);

// retorna 1 se deu certo, 0 se nao (repetido, nao existe ou arvore vazia)
int insere_ArvAVL(ArvAVL *raiz, int valor);
int remove_ArvAVL(ArvAVL *raiz, int valor);
int consulta_ArvAVL(ArvAVL *raiz, int valor);

int  altura_ArvAVL(ArvAVL *raiz);   // vazia = -1
int  totalNO_ArvAVL(ArvAVL *raiz);
int  estaVazia_ArvAVL(ArvAVL *raiz);

void emOrdem_ArvAVL(ArvAVL *raiz);
int  emOrdemVetor_ArvAVL(ArvAVL *raiz, int *v, int max);
int  preOrdemVetor_ArvAVL(ArvAVL *raiz, int *v, int max);
void imprime_ArvAVL(ArvAVL *raiz);

// confere ordem, alturas e fb entre -1 e 1. se der erro escreve em msg
int verifica_ArvAVL(ArvAVL *raiz, char *msg, int tam);

EstatAVL estat_ArvAVL(void);
void     zeraEstat_ArvAVL(void);
long     totalRotacoes_ArvAVL(void);  // dupla conta como 2

#endif
