#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define TEMPO_SIMULACAO 100
#define NUM_FILOSOFOS 5

pthread_mutex_t garfos[NUM_FILOSOFOS];
pthread_t filosofos[NUM_FILOSOFOS];
int running = 1;

typedef struct {
    int qtdRefeicoes;
    double tempoTotalEsperado;
    double tempoMaxEsperado;

} filosofoMetricas;

filosofoMetricas metricas[NUM_FILOSOFOS];

double get_time() {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    return start.tv_sec + start.tv_nsec / 1e9;
}

void pensando(int filosofoID) {
    printf("Filósofo %d está pensando!\n", filosofoID);
    usleep(rand() % 2000000 + 5000000);

}

void comendo(int filosofoID) {
    printf("Filósofo %d está comendo!\n", filosofoID);
    usleep(rand() % 1000000 + 5000000);
    metricas[filosofoID].qtdRefeicoes++;

}

void* processamentoFilosofoSolucaoA (void* arg) {
    int ID = *(int*)arg;

    while(running) {
        pensando(ID);

        int esq = ID;
        int dir = (ID +1) % NUM_FILOSOFOS;

        int primeiro = (esq < dir) ? esq : dir;
        int segundo = (esq < dir) ? dir : esq;

        double inicioEspera = get_time();

        pthread_mutex_lock(&garfos[primeiro]);
        printf("Filósofo %d pegou o primeiro garfo.\n", ID);

        pthread_mutex_lock(&garfos[segundo]);

        double tempoEspera = get_time() - inicioEspera;

        metricas[ID].tempoTotalEsperado = tempoEspera;

        if(tempoEspera > metricas[ID].tempoMaxEsperado) {
            metricas[ID].tempoMaxEsperado = tempoEspera;
        }

        printf("Filósofo %d pegou ambos os garfos.\n", ID);

        comendo(ID);

        pthread_mutex_unlock(&garfos[primeiro]);
        pthread_mutex_unlock(&garfos[segundo]);
        printf("Filósofo %d liberou os garfos!\n", ID);
    }
    return NULL;
}



int main () {

    int IDs[NUM_FILOSOFOS];
    int qtdRefeicao = 0;
    double tempoMedio = 0;
    float mediaRef = 0;
    srand(time(NULL));



    for(int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_mutex_init(&garfos[i], NULL);
    }

    printf("Todos os garfos foram inicializados!\n");



    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        IDs[i] = i;
        if(pthread_create(&filosofos[i], NULL, processamentoFilosofoSolucaoA, &IDs[i] ) != 0) {
            perror("Falha ao criar thread.");
            return 1;
        }

        printf("Filósofo %d criado!\n", i);

    }

    sleep(TEMPO_SIMULACAO);
    running = 0;


    for(int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_join(filosofos[i], NULL);
    }



    for(int i = 0; i < NUM_FILOSOFOS; i++) {
        qtdRefeicao = qtdRefeicao + metricas[i].qtdRefeicoes;
        tempoMedio = metricas[i].qtdRefeicoes > 0 ? metricas[i].tempoTotalEsperado / metricas[i].qtdRefeicoes : 0;

        printf("Filósofo %d: %d refeições \n", i, metricas[i].qtdRefeicoes);
        printf("Tempo médio: %.2fs\n", tempoMedio);
        printf("Tempo máximo de espera: %.2fs\n", metricas[i].tempoMaxEsperado);
        printf("\n");
    }



    mediaRef = qtdRefeicao / NUM_FILOSOFOS;
    printf("Total de refeições: %d\n",qtdRefeicao);
    printf("Média de refeições: %.1f\n", mediaRef);




    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_mutex_destroy(&garfos[i]);
    }


    return 0;

}