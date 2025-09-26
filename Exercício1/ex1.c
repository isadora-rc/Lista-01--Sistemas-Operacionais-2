#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define DISTANCIA_CHEGADA 100 //distancia da corrida

int* posicoes_cavalos;
int num_cavalos;
int cavalo_vencedor = -1;
volatile bool corrida_acabou = false; // flag que sinaliza o fim da corrida

pthread_mutex_t mutex_placar;
pthread_mutex_t mutex_vencedor;
pthread_barrier_t barreira_largada;

// estrutura para passar dados para a thread de cada cavalo
typedef struct {
    int id;
    unsigned int seed; // thread-safe, gera num aleatorios
} CavaloArgs;



void imprimir_placar() {
    // trava o mutex para garantir que o placar nao seja alterado enquanto eh impresso
    pthread_mutex_lock(&mutex_placar);

    // codigos de escape ANSI para limpar a tela e mover o cursor para o inicio
    printf("\033[H\033[J");
    printf("--- PLACAR DA CORRIDA ---\n");

    for (int i = 0; i < num_cavalos; i++) {
        printf("Cavalo %02d: [", i + 1);
        int pos = posicoes_cavalos[i];
        for (int j = 0; j < DISTANCIA_CHEGADA; j++) {
            if (j < pos) {
                printf("=");
            } else {
                printf(" ");
            }
        }
        printf("]\n");
    }
    printf("-------------------------\n");

    //libera o mutex
    pthread_mutex_unlock(&mutex_placar);
}


void* funcao_cavalo(void* arg) {
    CavaloArgs* args = (CavaloArgs*) arg;
    int id_cavalo = args->id;

    //garantir uma largada sincronizada
    pthread_barrier_wait(&barreira_largada);

    while (!corrida_acabou) {

        usleep((rand_r(&args->seed) % 100 + 50) * 1000); //dorme entre 50ms e 150ms

        // calcula o avanco do passo (aleatorio)
        int passo = (rand_r(&args->seed) % 5) + 1; // Avança de 1 a 5 posições

        // --- Secao Critica: Atualizar posicao ---
        pthread_mutex_lock(&mutex_placar);
        posicoes_cavalos[id_cavalo] += passo;
        //garante que o cavalo nao ultrapasse a linha de chegada no placar visual
        if (posicoes_cavalos[id_cavalo] >= DISTANCIA_CHEGADA) {
            posicoes_cavalos[id_cavalo] = DISTANCIA_CHEGADA;
        }
        pthread_mutex_unlock(&mutex_placar);


        if (posicoes_cavalos[id_cavalo] >= DISTANCIA_CHEGADA) {
            pthread_mutex_lock(&mutex_vencedor);

            if (cavalo_vencedor == -1) {
                //vencedor
                cavalo_vencedor = id_cavalo;
                corrida_acabou = true;
            }
            //`corrida_acabou` = true.

            pthread_mutex_unlock(&mutex_vencedor);
        }
    }
    free(args); //libera a memoria alocada 
    pthread_exit(NULL);
}



int main() {
    printf("Quantos cavalos participarao da corrida? ");
    scanf("%d", &num_cavalos);

    if (num_cavalos <= 0) {
        printf("Numero de cavalos invalido.\n");
        return 1;
    }

    int aposta;
    printf("Aposte em um cavalo (de 1 a %d): ", num_cavalos);
    scanf("%d", &aposta);

    if (aposta <= 0 || aposta > num_cavalos) {
        printf("Aposta invalida.\n");
        return 1;
    }
    //converte a aposta (base 1) para o ID do cavalo (base 0)
    int id_aposta = aposta - 1;

    posicoes_cavalos = (int*) calloc(num_cavalos, sizeof(int));
    pthread_t* threads = (pthread_t*) malloc(num_cavalos * sizeof(pthread_t));

    //gerador aleatorio
    srand(time(NULL));

    //inicializa os mutexes e a barreira
    pthread_mutex_init(&mutex_placar, NULL);
    pthread_mutex_init(&mutex_vencedor, NULL);

    pthread_barrier_init(&barreira_largada, NULL, num_cavalos + 1);


    printf("Preparando os cavalos nos portoes de largada...\n");
    sleep(2);

    //cria as threads (cavalos)
    for (int i = 0; i < num_cavalos; i++) {
        CavaloArgs* args = (CavaloArgs*) malloc(sizeof(CavaloArgs));
        args->id = i;
        args->seed = rand();
        if (pthread_create(&threads[i], NULL, funcao_cavalo, (void*) args) != 0) {
            perror("Falha ao criar thread");
            return 1;
        }
    }

    printf("A CORRIDA VAI COMECAR!\n");
    sleep(1);

    //a thread main tambem espera na barreira. Quando ela passar, significa
    // que todas as outras threads tambem passaram dando a largada
    pthread_barrier_wait(&barreira_largada);


    //atualizar o placar na tela
    while (!corrida_acabou) {
        imprimir_placar();
        usleep(100000); //100ms
    }

    imprimir_placar();


    for (int i = 0; i < num_cavalos; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n--- FIM DA CORRIDA ---\n");
    printf("O grande vencedor eh o cavalo %d!\n", cavalo_vencedor + 1);

    if (cavalo_vencedor == id_aposta) {
        printf("Parabens! Voce ganhou a aposta!\n");
    } else {
        printf("Nao foi dessa vez...\n");
    }

    pthread_mutex_destroy(&mutex_placar);
    pthread_mutex_destroy(&mutex_vencedor);
    pthread_barrier_destroy(&barreira_largada);
    free(posicoes_cavalos);
    free(threads);

    return 0;
}