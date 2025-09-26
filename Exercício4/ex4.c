#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_ITENS 10 //itens pra processar
#define TAMANHO_FILA 5 
#define POISON_PILL -1 //fim

typedef struct {
    int buffer[TAMANHO_FILA];
    int quantidade;
    int inicio;
    int fim;
    pthread_mutex_t mutex;
    pthread_cond_t nao_vazia;
    pthread_cond_t nao_cheia;
} fila_t;

//inicializa a fila
void inicializar_fila(fila_t *f) {
    f->quantidade = 0;
    f->inicio = 0;
    f->fim = 0;
    pthread_mutex_init(&f->mutex, NULL);
    pthread_cond_init(&f->nao_vazia, NULL);
    pthread_cond_init(&f->nao_cheia, NULL);
}

//adiciona na fila
void enfileirar(fila_t *f, int item) {
    pthread_mutex_lock(&f->mutex);
    while (f->quantidade == TAMANHO_FILA)
        pthread_cond_wait(&f->nao_cheia, &f->mutex);

    f->buffer[f->fim] = item;
    f->fim = (f->fim + 1) % TAMANHO_FILA;
    f->quantidade++;

    pthread_cond_signal(&f->nao_vazia);
    pthread_mutex_unlock(&f->mutex);
}

//remove da fila
int desenfileirar(fila_t *f) {
    pthread_mutex_lock(&f->mutex);
    while (f->quantidade == 0)
        pthread_cond_wait(&f->nao_vazia, &f->mutex);

    int item = f->buffer[f->inicio];
    f->inicio = (f->inicio + 1) % TAMANHO_FILA;
    f->quantidade--;

    pthread_cond_signal(&f->nao_cheia);
    pthread_mutex_unlock(&f->mutex);
    return item;
}

fila_t fila_captura_processamento, fila_processamento_gravacao;

//thread de captura
void* thread_captura(void* arg) {
    for (int i = 1; i <= NUM_ITENS; i++) {
        printf("Captura: %d\n", i);
        enfileirar(&fila_captura_processamento, i);
        usleep(100000); // simula atraso de captura
    }
    //sinalizar fim
    enfileirar(&fila_captura_processamento, POISON_PILL);
    return NULL;
}

//thread de processamento
void* thread_processamento(void* arg) {
    while (1) {
        int item = desenfileirar(&fila_captura_processamento);
        if (item == POISON_PILL) {
            enfileirar(&fila_processamento_gravacao, POISON_PILL); // passa adiante
            break;
        }
        int resultado = item * 2; // exemplo de processamento
        printf("Processamento: %d -> %d\n", item, resultado);
        enfileirar(&fila_processamento_gravacao, resultado);
        usleep(150000); // simula tempo de processamento
    }
    return NULL;
}

//thread de gravacao
void* thread_gravacao(void* arg) {
    while (1) {
        int item = desenfileirar(&fila_processamento_gravacao);
        if (item == POISON_PILL)
            break;
        printf("Gravação: %d\n", item);
        usleep(120000); // simula tempo de gravacao
    }
    return NULL;
}


int main() {
    pthread_t t_captura, t_processamento, t_gravacao;

    inicializar_fila(&fila_captura_processamento);
    inicializar_fila(&fila_processamento_gravacao);

    pthread_create(&t_captura, NULL, thread_captura, NULL);
    pthread_create(&t_processamento, NULL, thread_processamento, NULL);
    pthread_create(&t_gravacao, NULL, thread_gravacao, NULL);

    pthread_join(t_captura, NULL);
    pthread_join(t_processamento, NULL);
    pthread_join(t_gravacao, NULL);

    printf("Processamento concluido sem deadlocks ou perdas.\n");
    return 0;
}