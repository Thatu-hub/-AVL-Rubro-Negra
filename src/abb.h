// abb.h - arvore binaria de busca sem balanceamento
// so serve pra comparar com as outras duas (com IDs em ordem ela vira uma lista)
// Thaliton do Nascimento Silva
#ifndef ABB_H
#define ABB_H

typedef struct NO_ABB *ArvABB;

ArvABB *cria_ArvABB(void);
void    libera_ArvABB(ArvABB *raiz);
int     insere_ArvABB(ArvABB *raiz, int valor);
int     consulta_ArvABB(ArvABB *raiz, int valor, long *comparacoes);
int     altura_ArvABB(ArvABB *raiz);

#endif
