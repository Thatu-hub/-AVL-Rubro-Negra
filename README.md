# Amazon Engineering Challenge: Árvores Balanceadas (AVL × Rubro-Negra)

Trabalho da Unidade II de **Estruturas de Dados Básicas II (IMD0039)**, IMD/UFRN, 2026.2.
**Autor:** Thaliton do Nascimento Silva

Protótipo em C de um registro de identificadores de encomendas para a Black Friday.
Na execução, o usuário escolhe entre uma **árvore AVL** e uma **árvore Rubro-Negra**
(variante caída para a esquerda, LLRB). Os algoritmos de balanceamento foram implementados
à mão, sem bibliotecas prontas.

## 1. Funcionalidades

As duas árvores oferecem as mesmas operações no menu do programa interativo:

- **Inserir** um ou vários IDs de encomenda (separados por espaço), mostrando as rotações
  (AVL) ou as rotações e trocas de cor (Rubro-Negra) feitas em cada inserção;
- **Remover** um ID, mostrando os ajustes de balanceamento;
- **Buscar** um ID, informando quantos nós foram visitados;
- **Listar em ordem** (percurso em ordem, IDs em ordem crescente);
- **Visualizar a árvore** desenhada no terminal. Na Rubro-Negra, cada nó mostra a cor:
  `[P]` preto e `[V]` vermelho (ou em cores, no terminal);
- **Verificar as propriedades** de balanceamento (fator de balanceamento na AVL; regras de cor
  e altura negra na Rubro-Negra);
- **Estatísticas**: altura, número de nós e total de rotações e trocas de cor;
- **Trocar de árvore** mantendo os mesmos IDs, para comparar as duas estruturas.

## 2. Decisões de implementação

| Situação | O que o programa faz |
|---|---|
| Inserir um ID que já existe | Ignora a inserção e avisa: `Encomenda 20 JÁ EXISTE: inserção ignorada (IDs são únicos)` |
| Buscar um ID inexistente | Informa `NÃO encontrada` e quantos nós visitou |
| Remover um ID inexistente | Informa `NÃO ENCONTRADA: árvore não foi alterada` |
| Remover de uma árvore vazia | Informa `A árvore está vazia: nada a remover` |
| Entrada que não é número | Mostra `Entrada inválida` e volta ao menu |
| Altura | Contada em arestas: árvore vazia = −1, folha = 0 |
| Rubro-Negra | Variante LLRB (Sedgewick): todo nó vermelho é filho à esquerda; inserção com 3 regras (rotação à esquerda, rotação à direita, troca de cor) |

## 3. Exemplo de uso

Escolhendo a Rubro-Negra (opção `2`), inserindo `30 20 10` e visualizando a árvore (opção `5`):

```
IDs separados por espaço: 30 20 10
  Encomenda 30 registrada.
  Balanceamento: nenhum ajuste necessário.
  Encomenda 20 registrada.
  Balanceamento: nenhum ajuste necessário.
  Encomenda 10 registrada.
  Balanceamento: 0 rotação(ões) à esquerda, 1 à direita, 1 troca(s) de cor

.-- 30[P]
20[P]
`-- 10[P]
Legenda: [P] = preto, [V] = vermelho
```

Na AVL (opção `1`), inserindo `10 20 30`:

```
  Encomenda 30 registrada.
  Balanceamento: 1 x RR (simples à esquerda)
```

Logs completos de demonstração das duas árvores estão em `resultados/demo_avl.txt` e
`resultados/demo_rubronegra.txt`.

---

Este guia mostra como compilar e rodar o código no **Visual Studio Code**.

---

## 4. Dependências

O código usa só a biblioteca padrão de C (C99); não há bibliotecas externas.

**Versão compilada:** a pasta `bin/` já traz os executáveis prontos
(`amazon_arvores.exe`, `testes.exe`, `benchmark.exe` e `simulacao.exe` para Windows;
os mesmos nomes sem `.exe` para Linux). Para só usar o programa, basta rodar
`./bin/amazon_arvores` (veja a seção 7). Para compilar de novo, você precisa de:

1. **Visual Studio Code**, com a extensão **C/C++** (Microsoft). Ao abrir a pasta, o VS Code sugere instalar.
2. **Compilador gcc**:
   - **Windows:** instale o [MSYS2](https://www.msys2.org/), abra o terminal *MSYS2 UCRT64* e rode
     `pacman -S mingw-w64-ucrt-x86_64-gcc`. Depois, adicione `C:\msys64\ucrt64\bin` à variável de
     ambiente **Path** do Windows e reabra o VS Code.
   - **Linux (Ubuntu/Debian):** `sudo apt install build-essential`
   - **macOS:** `xcode-select --install`

Para conferir, abra o terminal do VS Code (**Terminal → Novo Terminal**) e digite `gcc --version`.

## 5. Abrir o projeto

**Arquivo → Abrir Pasta…** e escolha a pasta `amazon-arvores` (a que contém `src/` e este README).
Todos os comandos abaixo são executados a partir dessa pasta.

```
amazon-arvores/
├── src/          código-fonte comentado
│   ├── avl.c / avl.h                 árvore AVL
│   ├── rubronegra.c / rubronegra.h   árvore Rubro-Negra (LLRB)
│   ├── abb.c / abb.h                 ABB sem balanceamento (só para comparação)
│   ├── main.c                        programa interativo (menu)
│   ├── testes.c                      testes automáticos
│   ├── benchmark.c                   análise comparativa
│   └── simulacao.c                   simulação da Black Friday
├── bin/          executáveis compilados (Windows e Linux)
├── exemplos/     roteiros de entrada para demonstração
├── resultados/   saídas usadas na apresentação (CSV e logs)
└── .vscode/      tarefas prontas do VS Code
```

## 6. Compilar

**Pelo VS Code (mais fácil):** aperte **Ctrl+Shift+B** (*Compilar tudo*). Os quatro programas são gerados em `bin/`.

**Pelo terminal** (funciona no PowerShell, no Linux e no macOS):

```
gcc -Wall -Wextra -O2 -std=c99 -o bin/amazon_arvores src/main.c src/avl.c src/rubronegra.c
gcc -Wall -Wextra -O2 -std=c99 -o bin/testes src/testes.c src/avl.c src/rubronegra.c
gcc -Wall -Wextra -O2 -std=c99 -o bin/benchmark src/benchmark.c src/avl.c src/rubronegra.c src/abb.c
gcc -Wall -Wextra -O2 -std=c99 -o bin/simulacao src/simulacao.c src/avl.c src/rubronegra.c src/abb.c -lm
```

No Linux/macOS (ou no Windows com `make` instalado) também dá para usar `make`.

## 7. Rodar

**Pelo VS Code:** **Terminal → Executar Tarefa…** e escolha uma das tarefas `Rodar: …`.
Cada tarefa compila o programa antes de rodar.

**Pelo terminal:**

| Programa | Comando | O que faz |
|---|---|---|
| Programa interativo | `./bin/amazon_arvores` | Menu: escolha AVL ou Rubro-Negra e insira, remova, busque, liste e visualize a árvore |
| Testes | `./bin/testes` | Bateria de testes automáticos nas duas árvores (`./bin/testes -v` mostra os detalhes) |
| Benchmark | `./bin/benchmark 5` | Compara as árvores com 1 mil, 10 mil e 100 mil IDs (5 repetições) e grava `resultados/experimento1.csv` e `experimento2.csv` |
| Simulação (série) | `./bin/simulacao` | Simula a Black Friday com 10 mil a 200 mil pedidos e grava `resultados/simulacao_serie.csv` |
| Simulação (10 milhões) | `./bin/simulacao 10000000` | Rodada grande: cerca de 1 minuto e 1 GB de memória; grava `resultados/simulacao_blackfriday.csv` |

### Usando o programa interativo

1. Digite `1` para AVL ou `2` para Rubro-Negra.
2. No menu, use `1` para inserir (vários IDs separados por espaço, por exemplo `30 20 10`),
   `3` para buscar, `2` para remover, `5` para ver o desenho da árvore e `7` para ver altura e rotações.
3. `10` troca de árvore mantendo os mesmos IDs; `0` sai.

Se o terminal mostrar códigos estranhos no lugar das cores da Rubro-Negra, rode `./bin/amazon_arvores --sem-cor`
(a cor aparece como `[P]` preto ou `[V]` vermelho).

### Rodar um roteiro de exemplo (opcional)

Os arquivos de `exemplos/` contêm entradas prontas para o programa interativo:

- **Linux/macOS:** `./bin/amazon_arvores --sem-cor --eco < exemplos/roteiro_avl.txt`
- **Windows (PowerShell):** `Get-Content exemplos/roteiro_avl.txt | ./bin/amazon_arvores --sem-cor --eco`

## 8. Sobre os resultados

- As contagens (altura, nós visitados, rotações) são sempre as mesmas, em qualquer computador:
  os dados de teste usam uma semente fixa.
- Os **tempos em segundos** mudam de máquina para máquina. Os valores da apresentação estão em `resultados/`.
- Rodar o benchmark ou a simulação **sobrescreve** os CSVs de `resultados/` com os tempos do seu computador.

## 9. Problemas comuns

- **`gcc` não é reconhecido:** o compilador não está no Path. Refaça a seção 4 e reabra o VS Code.
- **Acentos estranhos no terminal do Windows:** os programas já ativam UTF-8; se ainda aparecer errado,
  rode `chcp 65001` no terminal antes.
- **`./bin/...` não encontrado:** compile primeiro (seção 6) e confira se o terminal está na pasta do projeto.
