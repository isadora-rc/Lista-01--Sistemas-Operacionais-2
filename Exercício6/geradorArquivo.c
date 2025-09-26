#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main (int argc, char *argv[]) {

    int valor;


    if ( argc < 3) {
        printf("Uso: %s <nome_do_arquivo> <numero_de_inteiros>\n", argv[0]);
        return 1;
    }

    const char *nome_arquivo = argv[1];
    long long int qtd_inteiros = atoll(argv[2]);

    FILE *arq;

    arq = fopen(nome_arquivo, "wb");
    if (arq == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    srand((unsigned)time(NULL));


    for(long long int i = 0; i < qtd_inteiros; i++) {
        valor = rand() % 1000;
        fwrite(&valor, sizeof(int), 1, arq);
    }

    fclose(arq);

    printf("Arquivo %s criado com %lld inteiros.\n", nome_arquivo, qtd_inteiros);

    return 0;
}
