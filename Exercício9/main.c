#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>


#define TAM_EQUIPE 4 // k threads que compõem a equipe de revezamento
#define DURACAO_TESTE_SEGUNDOS 120 // duração da amostra de tempo para o teste de desempenho


typedef struct {
    int tamEquipe;
    int roundAtual;
    pthread_barrier_t barrier;
    pthread_mutex_t roundMutex;
    volatile int running;
} InfoCorrida;

typedef struct {
    int idCorredor;
    int idEquipe;
    InfoCorrida *infocorrida;
} InfoCorredor;

void simularPerna() {
    usleep((rand() % 200000) + 100000);

}

void* processamentoCorredor (void* arg){
    InfoCorredor *corredor = (InfoCorredor*)arg;
    InfoCorrida *corrida = corredor->infocorrida;

    printf("Corredor %d da equipe %d iniciou.\n", corredor->idCorredor, corredor->idEquipe);

    while(corrida->running){
        simularPerna();

        printf("Corredor %d (equipe %d) chegou na barreira.\n", corredor->idCorredor, corredor->idEquipe);

        int resultadoBarreira = pthread_barrier_wait(&corrida->barrier);

        if(resultadoBarreira ==  PTHREAD_BARRIER_SERIAL_THREAD) {
            pthread_mutex_lock(&corrida->roundMutex);
            corrida->roundAtual++;
            printf("Rodada %d concluída.\n", corrida->roundAtual);
            pthread_mutex_unlock(&corrida->roundMutex);
        }

        usleep(3);
    }

    printf("Corredor %d da equipe %d finalizou a corrida!\n", corredor->idCorredor, corredor->idEquipe);
    return NULL;
}

int main() {
    srand(time(NULL));

    InfoCorrida corrida;
    corrida.tamEquipe = TAM_EQUIPE;
    corrida.roundAtual = 0;
    corrida.running = 1;

    if (pthread_barrier_init(&corrida.barrier, NULL, TAM_EQUIPE) != 0) {
        perror("Erro ao inicializar a barreira");
        return 1;
    }

    pthread_mutex_init(&corrida.roundMutex, NULL);

    pthread_t threads[TAM_EQUIPE];
    InfoCorredor infoCorredores[TAM_EQUIPE];

    for (int i = 0; i < TAM_EQUIPE; i++) {
        infoCorredores[i]. idCorredor = i;
        infoCorredores[i].idEquipe = 1;
        infoCorredores[i].infocorrida = &corrida;

        pthread_create(&threads[i], NULL, processamentoCorredor, &infoCorredores[i]);

    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    sleep(DURACAO_TESTE_SEGUNDOS);

    corrida.running = 0;
    clock_gettime(CLOCK_MONOTONIC, &end);

    for (int i = 0; i < TAM_EQUIPE; i++) {
        pthread_join(threads[i], NULL);
    }


    double tempoEx = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    double rodadasMin = (double)corrida.roundAtual / tempoEx * 60.0;

    printf("Tamanho da equipe: %d\n", TAM_EQUIPE);
    printf("Rodadas totais concluídas: %d\n", corrida.roundAtual);
    printf("Tempo real decorrido: %.2f\n", tempoEx);
    printf("Rodadas concluídas por minuto: %.2f\n", rodadasMin);


    pthread_mutex_destroy(&corrida.roundMutex);
    pthread_barrier_destroy(&corrida.barrier);
    return 0;
}