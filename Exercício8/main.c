#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NITEMS 10000           // total de itens por produtor
#define BURST_SIZE 50          // itens produzidos em sequ\u00eancia (rajada)
#define IDLE_TIME 50000        // microssegundos de ociosidade
#define HIGH_WATERMARK 0.8     // 80% do buffer cheio = backpressure
#define MONITOR_INTERVAL 100000 // microssegundos entre logs

typedef struct {
    int id;
    struct timespec ts_prod;
} item_t;

item_t *buffer;
int N;
int in = 0, out = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t empty, full;

int n_producers, n_consumers;
long produced = 0, consumed = 0;
volatile int producers_finished = 0;
double total_wait = 0;

// retorna ocupa\u00e7\u00e3o do buffer
int buffer_occupancy() {
    pthread_mutex_lock(&mutex);
    int occ = produced - consumed;
    pthread_mutex_unlock(&mutex);
    return occ;
}

void *producer(void *arg) {
    int id = *(int *)arg;
    int produced_local = 0;

    while (produced_local < NITEMS) {
        // backpressure
        while ((double)buffer_occupancy() / N >= HIGH_WATERMARK) {
            usleep(1000); // espera curta
        }

        // produz at\u00e9 BURST_SIZE ou at\u00e9 acabar os itens desse produtor
        int lote = BURST_SIZE;
        if (NITEMS - produced_local < BURST_SIZE)
            lote = NITEMS - produced_local;

        for (int j = 0; j < lote; j++) {
            item_t it;
            it.id = id * NITEMS + produced_local;
            clock_gettime(CLOCK_MONOTONIC, &it.ts_prod);

            sem_wait(&empty);
            pthread_mutex_lock(&mutex);

            buffer[in] = it;
            in = (in + 1) % N;
            produced++;
            produced_local++;

            pthread_mutex_unlock(&mutex);
            sem_post(&full);
        }

        // ociosidade
        usleep(IDLE_TIME);
    }
    return NULL;
}

void *consumer(void *arg) {
    (void)arg;
    while (1) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        if (producers_finished && consumed >= n_producers * NITEMS) {
            pthread_mutex_unlock(&mutex);
            sem_post(&full);
            break;
        }

        item_t it = buffer[out];
        out = (out + 1) % N;
        consumed++;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);

        struct timespec ts_cons;
        clock_gettime(CLOCK_MONOTONIC, &ts_cons);

        double wait = (ts_cons.tv_sec - it.ts_prod.tv_sec) +
                      (ts_cons.tv_nsec - it.ts_prod.tv_nsec) / 1e9;

        pthread_mutex_lock(&wait_mutex);
        total_wait += wait;
        pthread_mutex_unlock(&wait_mutex);

        usleep(rand() % 500);
    }
    return NULL;
}

// Monitor imprime ocupa\u00e7\u00e3o direto no terminal
void *monitor(void *arg) {
    (void)arg;
    while (!producers_finished || consumed < n_producers * NITEMS) {
        int occ = buffer_occupancy();
        printf("Buffer ocupacao: %d / %d\n", occ, N);
        fflush(stdout);
        usleep(MONITOR_INTERVAL);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: %s <tam_buffer> <produtores> <consumidores>\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]);
    n_producers = atoi(argv[2]);
    n_consumers = atoi(argv[3]);

    buffer = malloc(N * sizeof(item_t));
    if (!buffer) {
        perror("Erro ao alocar buffer");
        return 1;
    }

    sem_init(&empty, 0, N);
    sem_init(&full, 0, 0);

    pthread_t prod[n_producers], cons[n_consumers], mon;
    int ids[n_producers];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_create(&mon, NULL, monitor, NULL);

    for (int i = 0; i < n_producers; i++) {
        ids[i] = i;
        pthread_create(&prod[i], NULL, producer, &ids[i]);
    }

    for (int i = 0; i < n_consumers; i++)
        pthread_create(&cons[i], NULL, consumer, NULL);

    for (int i = 0; i < n_producers; i++)
        pthread_join(prod[i], NULL);

    producers_finished = 1;
    for (int i = 0; i < n_consumers; i++)
        sem_post(&full);

    for (int i = 0; i < n_consumers; i++)
        pthread_join(cons[i], NULL);

    pthread_join(mon, NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double total_time = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / 1e9;
    double throughput = (double)consumed / total_time;
    double avg_wait = total_wait / consumed;

    printf("\n==== RESULTADOS ====\n");
    printf("Tamanho do Buffer: %d\n", N);
    printf("Produtores: %d, Consumidores: %d\n", n_producers, n_consumers);
    printf("Itens produzidos: %ld, Itens consumidos: %ld\n", produced, consumed);
    printf("Tempo Total: %.4f s\n", total_time);
    printf("Throughput: %.2f itens/s\n", throughput);
    printf("Tempo Medio de Espera (no buffer): %.6f s\n", avg_wait);

    free(buffer);
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&wait_mutex);

    return 0;
}