#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

//Definição de limites dos valores para o aruivo
#define HISTOGRAMA_TAM 1000

long long somaTotal = 0;
int histogramaTotal[HISTOGRAMA_TAM] = {0};

pthread_mutex_t  mutexSoma = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexHistograma = PTHREAD_MUTEX_INITIALIZER;


//Definição da estrutura de Threads
typedef struct {
    int thread_id; // id da thread
    long long start_offset; // inicio do bloco em bytes
    long long end_offset; // fim do bloco em bytes
    const char *arq; // nome do arquivo para a leitura
} ParamThreads;



//Função que calcula o tamanho do arquivo para que cada thread tenha seu bloco de leitura definido
long long int tamanhoArquivo (const char *nome_arq) {
    long long int tam = -1;
    FILE *arq;
    arq = fopen(nome_arq, "rb");
    if (arq == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    fseek(arq, 0, SEEK_END);

    tam = ftell(arq);

    rewind(arq);
    fclose(arq);
    return tam;
}


//Função que processa as Threads - Lê os inteiros e faz a soma local
void *processamentoThread(void *arg) {
    long long int somaThread = 0;
    int histogramaThread[HISTOGRAMA_TAM] = {0};
    int inteiro;
    long long bytesLidos = 0;

    ParamThreads *params = (ParamThreads *) arg;


    long long start = params->start_offset;
    long long end = params->end_offset;
    const char *nome_arq = params->arq;

    FILE *arq;
    arq = fopen(nome_arq, "rb");
    if (arq == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return NULL;
    }

    fseek(arq, start, SEEK_SET);


    while (bytesLidos < (end - start)) {
        fread(&inteiro, sizeof (int), 1, arq);

        somaThread = somaThread + inteiro;
        if(inteiro >= 0 && inteiro < HISTOGRAMA_TAM) {
            histogramaThread[inteiro]++;
        }

        bytesLidos = bytesLidos + sizeof (int);
    }

    fclose(arq);

    pthread_mutex_lock(&mutexSoma);
    somaTotal = somaTotal + somaThread;
    pthread_mutex_unlock(&mutexSoma);

    pthread_mutex_lock(&mutexHistograma);
    for(int i = 0; i < HISTOGRAMA_TAM; i++) {
        histogramaTotal[i] = histogramaTotal[i] + histogramaThread[i];
    }
    pthread_mutex_unlock(&mutexHistograma);

    return NULL;

}


int main (int argc, char *argv[]) {

   long long int tamTotal;
   long long int valorBloco;


    if (argc < 3) {
        printf("Uso: %s <nome_do_arquivo> <qtd_threads>\n", argv[0]);
        return 1;
    }

    const char *nome_arq = argv[1];
    int qtdThreads = atoll(argv[2]);


    pthread_t threads[qtdThreads];
    ParamThreads param[qtdThreads];


    tamTotal = tamanhoArquivo(nome_arq);
    valorBloco = tamTotal/qtdThreads;


    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < qtdThreads; i++) {

        param[i].thread_id = i;
        param[i].start_offset = i * valorBloco;
        param[i].arq = nome_arq;

        if (i == (qtdThreads - 1)) {
            param[i].end_offset = tamTotal;

        }
        else {
            param[i].end_offset = (i + 1) * valorBloco;
        }


        pthread_create(&threads[i], NULL, processamentoThread, (void *)&param[i]);


    }

    for (int i = 0; i < qtdThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double tempo = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Tempo de execução: %.6f segundos.\n", tempo);
    printf("Soma total: %lld\n", somaTotal);
    printf("Histograma de frequências\n");
    for(int i = 0; i < HISTOGRAMA_TAM; i++) {
        if (histogramaTotal[i] > 0) {
            printf("%d %d \n", i, histogramaTotal[i]);
        }
    }

    return 0;

}