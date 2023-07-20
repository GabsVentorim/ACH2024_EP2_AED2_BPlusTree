#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**************************************** INFOS ********************************************
 * Nome: Gabriel Ventorim Silva
 * Nº USP: 13672147
 ********************************************************************************************/

#define t 3 // fixado pelo exercício

typedef int TipoChave;
 
typedef struct s
{
    TipoChave chave[2 * t - 1]; // num de epaços no vetor
    struct s *filhos[2 * t];    // num de ponteiros
    int numChaves;              // num de ch armazenadas neste nó
    bool folha;                 // true se nao tem mais filhos
    struct s *proxFolha;        // aponta para a folha irma a direita
} NO;

typedef struct
{
    NO *raiz;
} ArvBM; // B-Plus Tree

// min de num é t-1 em um nó não folha e deve conter pelo menos t filhos

/**************************************** METODOS *********************************************/

NO *criaNo();
void iniciaArv(ArvBM *T);
void splitFilho(NO *x, int i, NO *y);
void inserirRaizNaoCheio(NO *x, int ch);
void inserir(ArvBM *T, int ch);
void remover(ArvBM *T, int ch);
void printArv(NO *x);
void mergeFilhos(ArvBM *T, NO *x, int i);
void removeSubArv(ArvBM *T, NO *x, int ch);
void removeArvore(ArvBM *T, int ch);
void removerFolha(NO *x, int ch);
void pegaDaEsq(ArvBM *T, NO *x, int n);
void pegaDaDir(ArvBM *T, NO *x, int n);

void printArv(NO *x)
{
    if (x->numChaves == 0) // arv vazia
    {
        printf("vazia\n");
    }

    else
    {
        int i = 0;

        if (x->folha)
        {
            printf("(");
            while (i < x->numChaves)
            {
                printf("%d", x->chave[i]);
                if (i < x->numChaves - 1)
                    printf(" ");
                i++;
            }
            printf(")");
            return;
        }

        else
        {

            printf("(");

            while (i <= x->numChaves)
            {
                printArv(x->filhos[i]);
                if (i < x->numChaves)
                    printf(" %d ", x->chave[i]);
                else
                    printf(")");
                i++;
            }
        }

        // if (!(x->folha)) // descendo para os filhos se nao for folha
        //     printArv(x->filhos[i]);
    }
}

NO *criaNo()
{
    NO *novo;
    if (!(novo = (NO *)malloc(sizeof(NO))))
    {
        fprintf(stderr, "Memoria insuficiente!\n");
        return NULL;
    }
    int i;
    for (i = 0; i < (2 * t); i++) // filhos apontando para null
        novo->filhos[i] = NULL;
    novo->numChaves = 0;
    novo->proxFolha = NULL;
    novo->folha = true; // eh folha
    return novo;
}

void iniciaArv(ArvBM *T)
{
    T->raiz = criaNo();
}

// talvez colocar um auxiliar para saber em qual o no esta
// x: noh raiz da arv
int busca(NO *x, int ch)
{
    int i = 0;
    while ((i < x->numChaves) && (ch > x->chave[i]))
        i++;
    if ((i < x->numChaves) && (ch == x->chave[i]))
        return i;
    if (x->folha)
        return -1;
    else
        return busca(x->filhos[i], ch);
}

/* @param x noh pai interno nao cheio que receverah mais um valor
 * @param i posicao em x onde a chave subirah
 * @param y noh filho que serah dividido
 * Acesso ao disco -> O(1)
 * CPU -> O(t)
 */
void splitFilho(NO *x, int i, NO *y)
{
    NO *z = criaNo();
    z->folha = y->folha; // z deve ser o q y eh
    int j;               // iterador
    if (!(y->folha))     // folhas nao tem filhos
    {
        z->numChaves = t - 1; // divisao do num de filhos para z

        for (j = 0; j < t; j++) // passando filhos de y para z
            z->filhos[j] = y->filhos[t + j];

        for (j = 0; j < t - 1; j++) // passando os de y para z
            z->chave[j] = y->chave[j + t];
    }
    else // B+: se for folha, atualiza as proxs de y e z
    {
        z->numChaves = t; // divisao do num de filhos para z

        z->proxFolha = y->proxFolha; // atualizando prox Folha
        y->proxFolha = z;            // atualizando prox Folha

        for (j = 0; j < t; j++)
            z->chave[j] = y->chave[j + t - 1];
    }

    y->numChaves = t - 1; // mudando num de chaves em y, sendo folha ou nao

    for (j = x->numChaves; j >= i + 1; j--) // passando os filhos de x para o lado
        x->filhos[j + 1] = x->filhos[j];

    x->filhos[i + 1] = z; // passando o filho de x em i para z

    for (j = x->numChaves - 1; j >= i; j--)
        x->chave[j + 1] = x->chave[j]; // passando valores de x para p lado

    x->chave[i] = y->chave[t - 1];   // subindo o valor
    x->numChaves = x->numChaves + 1; // aumentando o num de chaves de x
}

// x: eh o no raiz da arvore -> colocar na chamda T->raiz
void inserirRaizNaoCheio(NO *x, int ch)
{
    int i = x->numChaves; // iterador
    if (x->folha)         // se for folha para
    {
        while ((i > 0) && (ch < x->chave[i - 1])) // ainda no vetor e chaves menore que ch
        {
            x->chave[i] = x->chave[i - 1]; // passando para o lado
            i--;
        }
        x->chave[i] = ch;                // fazendo a insercao
        x->numChaves = x->numChaves + 1; // aumentando o num de chaves
    }
    else // senão for folha
    {
        while ((i > 0) && ch < x->chave[i - 1]) // achar filho para descer
            i--;

        if (x->filhos[i]->numChaves == (2 * t - 1))
        {
            splitFilho(x, i, x->filhos[i]); // no cheio
            if (ch > x->chave[i])
                i++; // correção para qual filho descer se ch ainda for o menor
        }
        inserirRaizNaoCheio(x->filhos[i], ch); // fazer recursao para buscar o lugar certo
    }
}

void inserir(ArvBM *T, int ch)
{
    NO *r = T->raiz;
    if (r->numChaves == 2 * t - 1) // raiz estah cheio
    {
        NO *s = criaNo();
        T->raiz = s;
        s->numChaves = 0; // vai somar na funcao splitFilho
        s->folha = false;
        s->filhos[0] = r;
        splitFilho(s, 0, r);        // fazer um split para mudar a raiz de T
        inserirRaizNaoCheio(s, ch); // faz a insercao
    }
    else // raiz nao cheia
    {
        inserirRaizNaoCheio(r, ch); // faz a insercao
    }
}

// void remover(ArvBM *T, int ch)
// {
//     NO *r = T->raiz;
//     if (T->raiz == NULL || (T->raiz->numChaves == 0)) // arvore vazia
//         return;
//     else
//         removerRecursivo(T->raiz, ch);
//     if (r->numChaves == 0) // se a raiz ficou vazia
//     {
//         if (r->folha) // se a raiz for folha
//             T->raiz = NULL;
//         else                        // se nao for folha
//             T->raiz = r->filhos[0]; // nova raiz
//         free(r);                    // liberando a raiz antiga
//     }
// }

// a remocao deve ver os filhos entre ch: trocar com um ou outro (se tem tam suf para perser um) ou junta-los
// nao estou vendo os filhos de y e z
/* @param x noh interno que contem ch
 * @param i posicao em x onde a chave estah
 * @param y noh filho de x valor que sera trocado por ch
 * @param ch valor da chave que sera removida
 */
void remove2ab(NO *x, int i, NO *y, TipoChave ch)
{
    int j;
    x->chave[i] = y->chave[y->numChaves - 1]; // trocando ch por o maior valor de y
    y->chave[y->numChaves - 1] = ch;          // trocando o valor de y com ch
    // nao se faco agora: y->numChaves = y->numChaves - 1;          // diminuindo o num de chaves de y
}

// x deve ser interno
// nohs y e z tem menos de t - 1 chaves
// z = y->proxFolha

/* @param x noh interno que contem ch
 * @param i posicao em x onde a chave estah;
 * @param y noh filho de x valor; x->filhos[i] = y
 * @param z noh filho de x valor; x->filhos[i + 1] = y
 * @param ch valor da chave que sera removida
 */
void remove2c(NO *x, int i, NO *y, NO *z, TipoChave ch)
{
    int j;
    // arrumando x
    for (j = i; j < x->numChaves; j++) // passando os filhos para o lado
        x->filhos[j] = x->filhos[j + 1];

    for (j = i; j < x->numChaves - 1; j++)
        x->chave[j] = x->chave[j + 1]; // passando os valores de x para o lado
    x->numChaves = x->numChaves - 1;   // diminuindo o num de chaves de x

    // unindo y e z
    y->chave[y->numChaves] = ch; // colocando ch no final de y
    for (j = 0; j < t - 1; j++)  // passando os valores de z para y
        y->chave[t + j] = z->chave[j];

    if (!(y->folha)) // se nao for folha
    {
        for (j = 0; j < t; j++) // passando os filhos de z para y
            y->filhos[t + j] = z->filhos[j];
    }
}

void mergeFilhos(ArvBM *T, NO *x, int n)
{
    // printf("merge\n");
    NO *esq = x->filhos[n];
    NO *dir = x->filhos[n + 1];
    NO *dir2 = x->filhos[n + 2];
    // printf(">>>>>>>n: %d\n", n);
    int i;
    if (esq->folha)
    {

        for (i = 0; i < dir->numChaves; i++)
        {
            esq->chave[esq->numChaves] = dir->chave[i];
            esq->numChaves++;
        }
        //     inserir(T, dir->chave[i]); // inserir(esq, dir->chave[i]);
        //     // x->chave[n] = dir->chave[i];
        //     printArv(T->raiz);
    }
    else
    {
        esq->chave[esq->numChaves] = x->chave[n]; // esq->chave[esq->numChaves - 1] = x->chave[n];
        esq->numChaves++;
        for (i = 0; i < dir->numChaves; i++)
        {
            esq->chave[esq->numChaves] = dir->chave[i];
            esq->filhos[esq->numChaves] = dir->filhos[i];
            esq->numChaves++;
        }
        esq->filhos[esq->numChaves] = dir->filhos[dir->numChaves];
    }

    // arrumando x
    for (i = n + 1; i < x->numChaves; i++)
    {
        x->chave[i - 1] = x->chave[i];
        x->filhos[i] = x->filhos[i + 1];
    }
    x->filhos[n + 1] = dir2;
    x->numChaves--;
    // printf(">>>> aquiii %d\n", esq->numChaves);

    free(dir);
    // removeArvore(T, x->chave[n]);
    // printArv(T->raiz);
}

void removerFolha(NO *x, int ch)
{
    // printf("remFolha\n");

    int j, i = 0;
    while (i < x->numChaves && ch > x->chave[i])
    {
        i++;
    }
    for (j = i; j < x->numChaves - 1; j++)
    {
        // //printf("%d\n", x->chave[j]);
        x->chave[j] = x->chave[j + 1];
    }
    x->numChaves = x->numChaves - 1;
    // //printf("%d\n", x->chave[j]);
}

void pegaDaEsq(ArvBM *T, NO *x, int n)
{
    // printf("esq\n");

    int i;
    NO *esq = x->filhos[n];
    NO *dir = x->filhos[n + 1];
    if (esq->folha)
    {
        x->chave[n] = esq->chave[esq->numChaves - 1];
        removerFolha(esq, x->chave[n]);
        // printArv(T->raiz);
        inserir(T, x->chave[n]); // inserir(dir, x->chave[n]);
    }
    else
    {
        for (i = dir->numChaves; i > 0; i++)
        {
            dir->chave[i] = dir->chave[i - 1];
            dir->filhos[i] = dir->filhos[i - 1];
        }
        dir->filhos[1] = dir->filhos[0];
        dir->filhos[0] = esq->filhos[esq->numChaves];
        dir->chave[0] = x->chave[n];
        dir->numChaves++;
        x->chave[n] = esq->chave[esq->numChaves - 1];
        esq->numChaves--;
    }
}

void pegaDaDir(ArvBM *T, NO *x, int n)
{
    // printf("dir\n");

    int i;
    NO *esq = x->filhos[n];
    NO *dir = x->filhos[n + 1];
    if (esq->folha)
    {
        inserir(T, x->chave[n]); // inserir(esq, x->chave[n]);
        removerFolha(dir, dir->chave[0]);
        x->chave[n] = dir->chave[0];
    }
    else
    {
        esq->chave[esq->numChaves] = x->chave[n];
        esq->numChaves++;
        esq->filhos[esq->numChaves] = dir->filhos[0];
        x->chave[n] = dir->chave[0];
        for (i = 0; i < esq->numChaves - 1; i++)
        {
            dir->chave[i] = dir->chave[i + 1];
            dir->filhos[i] = dir->filhos[i + 1];
        }
        dir->filhos[dir->numChaves - 1] = dir->filhos[dir->numChaves];
        dir->numChaves--;
    }
}

void removeSubArv(ArvBM *T, NO *x, int ch)
{
    // printf("subArv\n");

    int i = 0;
    while (i < x->numChaves && ch >= x->chave[i])
        i++;

    if ((i > 0) && (x->chave[i - 1] == ch))
    {
        if (x->folha)
        {
            // printf("---> %d\n", ch);
            removerFolha(x, ch);
        }

        else
        {
            NO *ant = x;
            NO *p = x->filhos[i];
            while (p) // da para mudar ate ser folha
            {
                ant = p;
                p = p->filhos[0];
            }
            x->chave[i - 1] = ant->chave[1]; // trocando o valor
        }
        // printf("-> %d\n", ch);

        if (!(x->folha) && (x->filhos[i]->numChaves == t - 1))
        {
            if ((i > 0) && (x->filhos[i - 1]->numChaves >= t))
            {
                // printf("---> %d\n", ch);
                pegaDaEsq(T, x, i - 1);
            }
            else if ((i < x->numChaves) && (x->filhos[i + 1]->numChaves >= t))
                pegaDaDir(T, x, i);
            else
            {
                // printf("-> -> %d\n", ch);
                mergeFilhos(T, x, i - 1);
                i--;
            }
        }
        if (!(x->folha))
            removeSubArv(T, x->filhos[i], ch);
    }
    else
    {
        if (x->filhos[i]->numChaves == t - 1)
        {
            if ((i > 0) && (x->filhos[i - 1]->numChaves >= t))
            {
                pegaDaEsq(T, x, i - 1);
                if (!(x->folha))
                {
                    removeSubArv(T, x->filhos[i], ch);
                }
            }
            else if ((i < x->numChaves) && (x->filhos[i + 1]->numChaves >= t))
            {
                pegaDaDir(T, x, i);
                if (!(x->folha))
                {
                    removeSubArv(T, x->filhos[i], ch);
                }
            }
            else
            {
                if (i == x->numChaves)
                {
                    // printf("---> %d\n", ch);

                    mergeFilhos(T, x, i - 1);
                    i--;
                    if (!(x->folha))
                    {
                        removeSubArv(T, x->filhos[i], ch);
                    }
                }
                else
                {
                    // printf("-> ----> %d\n", ch);
                    mergeFilhos(T, x, i - 1);
                    if (!(x->folha))
                    {
                        // printf(">>>> aquii");
                        // printArv(T->raiz);
                        removeSubArv(T, x->filhos[i - 1], ch);
                    }
                }
            }
        }
        else
        {
            if (!(x->folha))
            {
                removeSubArv(T, x->filhos[i], ch);
            }
        }
    }
}

void removeArvore(ArvBM *T, int ch)
{
    // printf("remove\n");

    removeSubArv(T, T->raiz, ch);
    if (T->raiz->numChaves == 0)
    {
        NO *aux = T->raiz->filhos[0];
        free(T->raiz);
        T->raiz = aux;
    }
}

/**********************************************************************************************/

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Erro: quantidade de argumentos\n");
    }

    FILE *arq_ent = fopen(argv[1], "r");
    if (!arq_ent)
    {
        fprintf(stderr, "Erro: ponteiro do arq_ent\n");
    }

    // criar ArvBM T e deixa seu pont para root vazia
    ArvBM T;
    iniciaArv(&T);

    char acao = 'a';
    int valor = 0;

    while (acao != 'f')
    {
        fscanf(arq_ent, "%c %d ", &acao, &valor);

        if (acao == 'i')
        {
            // incluir
            printf("%c %d\n", acao, valor);
            inserir(&T, valor);
        }

        if (acao == 'r')
        {
            // remover
            printf("%c %d\n", acao, valor);
            // se nao tiver nos ele nao faz nada
            removeArvore(&T, valor);
        }

        if (acao == 'p')
        {
            // printar
            printf("%c\n", acao);
            printArv(T.raiz);
        }
    }
}

// switch (acao)
// {
// case 'i':
//     // incluir
//     printf("%c %d\n", acao, valor);
//     break;

// case 'r':
//     // remover
//     printf("%c %d\n", acao, valor);
//     break;

// case 'p':
//     // printar
//     printf("%c\n", acao);
//     break;

// default:
//     printf("Operacao invalida");
// }