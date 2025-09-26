#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct Tarefa {
    int valor; //valor a ser processado
    struct Tarefa* proximo; 
} Tarefa;


typedef struct FilaTarefas {//fila de tarefas concorrente

    Tarefa* frente;
    Tarefa* tras;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int fechada; //shutdown
} FilaTarefas;



void inicializarFila(FilaTarefas* fila) {
    fila->frente = fila->tras = NULL;
    pthread_mutex_init(&fila->mutex, NULL);
    pthread_cond_init(&fila->cond, NULL);
    fila->fechada = 0;
}

//desaloca a fila o mutex e a variavel de condicao
void destruirFila(FilaTarefas* fila) {

    pthread_mutex_destroy(&fila->mutex);
    pthread_cond_destroy(&fila->cond);
    
    Tarefa* atual = fila->frente;
    Tarefa* proximo;
    while(atual != NULL) {
        proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
}

//enfileira uma tarefa (thread-safe)
void enfileirar(FilaTarefas* fila, int valor) {
    //aloca a nova tarefa
    Tarefa* tarefa = malloc(sizeof(Tarefa));
    if (tarefa == NULL) {
        perror("Erro de alocacao de memoria");
        return;
    }
    tarefa->valor = valor;
    tarefa->proximo = NULL;

    //secao critica
    pthread_mutex_lock(&fila->mutex);
    if (fila->tras == NULL) {
        fila->frente = fila->tras = tarefa;
    } else {
        fila->tras->proximo = tarefa;
        fila->tras = tarefa;
    }
    pthread_cond_signal(&fila->cond); //acorda uma thread esperando
    pthread_mutex_unlock(&fila->mutex);
}

//retira uma tarefa da fila
Tarefa* desenfileirar(FilaTarefas* fila) {
    pthread_mutex_lock(&fila->mutex);
    
    while (fila->frente == NULL && !fila->fechada) { //espera por trabalho ou sinal de encerramento

        pthread_cond_wait(&fila->cond, &fila->mutex);
    }

    if (fila->frente == NULL && fila->fechada) { //condicao de saida

        pthread_mutex_unlock(&fila->mutex);
        return NULL; 
    }

    //desenfileiramento
    Tarefa* tarefa = fila->frente;
    fila->frente = tarefa->proximo;
    if (fila->frente == NULL) fila->tras = NULL;
    
    pthread_mutex_unlock(&fila->mutex);
    return tarefa;
}

void fecharFila(FilaTarefas* fila) {
    pthread_mutex_lock(&fila->mutex);
    fila->fechada = 1;
    pthread_cond_broadcast(&fila->cond); // Acorda TODAS as threads para terminarem
    pthread_mutex_unlock(&fila->mutex);
}


//CPU-bound: calculo de fibonacci
long fibonacci(int n) {
    if (n < 0) return -1; 
    if (n <= 1) return n;
    long a = 0, b = 1, c;
    for (int i = 2; i <= n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}


void* trabalhador(void* arg) {//threads worker

    FilaTarefas* fila = (FilaTarefas*)arg;
    Tarefa* tarefa;

    //enquanto houver tarefas para desenfileirar
    while ((tarefa = desenfileirar(fila)) != NULL) {
        long resultado = fibonacci(tarefa->valor);
        printf("[Thread %lu] Fibonacci(%d) = %ld\n", (unsigned long)pthread_self(), tarefa->valor, resultado);
        free(tarefa);
    }
    
    return NULL;
}


int main() {
    int N_THREADS = 4; //num. de threads do pool
    pthread_t threads[N_THREADS];
    FilaTarefas fila;

    inicializarFila(&fila);
    printf("Pool de threads iniciado com %d trabalhadores.\n", N_THREADS);

    //threads worker
    for (int i = 0; i < N_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, trabalhador, &fila) != 0) {
            perror("Erro ao criar thread");
        }
    }

    //produtor vai ler astarefas da entrada padrao ate EOF
    int valor;
    while (1) {
        printf("Digite 'n' para Fibonacci(n) (Ctrl+D para terminar): ");
        if (scanf("%d", &valor) != 1) {
            break; 
        }
        enfileirar(&fila, valor);
    }

    
    
    //sinaliza para os trabalhadores que n tem mais tarefas
    fecharFila(&fila);
    
    printf("Aguardando que as %d threads finalizem o processamento restante.\n", N_THREADS);
    
    //espera TODAS as threads terminem 
    for (int i = 0; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    destruirFila(&fila); 
    printf("Pool de threads finalizado e recursos liberados com sucesso.\n");

    return 0;
}