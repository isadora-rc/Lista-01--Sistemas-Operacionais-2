#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>

#define TEMPO_SIMULACAO 100
#define NUM_FILOSOFOS 5
#define MAX_MESA 4

typedef struct {
    int qtdRefeicoes;
    double tempoTotalEsperado;
    double tempoMaxEsperado;

} filosofoMetricas;

filosofoMetricas metricas[NUM_FILOSOFOS];
pthread_mutex_t garfos[NUM_FILOSOFOS];
pthread_t filosofos[NUM_FILOSOFOS];
sem_t salaJantar;
int running = 1;


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

void* processamentoFilosofoSolucaoB (void* arg) {

    int ID = *(int*)arg;

    while(running) {
        pensando(ID);

        double inicioEspera = get_time();

        printf("Filósofo %d quer entrar na sala de jantar (semáforo).\n", ID);
        sem_wait(&salaJantar);
        printf("Filósofo %d entrou na sala de jantar.\n", ID);

        int esq = ID;
        int dir = (ID + 1) % NUM_FILOSOFOS;


        pthread_mutex_lock(&garfos[esq]);
        printf("Filósofo %d pegou o primeiro garfo.(esq)\n", ID);

        pthread_mutex_lock(&garfos[dir]);

        double tempoEspera = get_time() - inicioEspera;

        metricas[ID].tempoTotalEsperado = tempoEspera;

        if (tempoEspera > metricas[ID].tempoMaxEsperado) {
            metricas[ID].tempoMaxEsperado = tempoEspera;
        }

        printf("Filósofo %d pegou ambos os garfos.(dir)\n", ID);


        comendo(ID);

        pthread_mutex_unlock(&garfos[dir]);
        pthread_mutex_unlock(&garfos[esq]);
        printf("Filósofo %d liberou os garfos!\n", ID);

        sem_post(&salaJantar);
        printf("Filósofo %d saiu da sala de jantar.\n", ID);

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
    printf("\n");

    sem_init(&salaJantar, 0, MAX_MESA);




    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        IDs[i] = i;
        if(pthread_create(&filosofos[i], NULL, processamentoFilosofoSolucaoB, &IDs[i] ) != 0) {
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
    sem_destroy(&salaJantar);


    return 0;

}



