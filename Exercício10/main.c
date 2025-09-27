#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>


#define TEMPO_INATIVIDADE_S 3


typedef struct {
    char id[10];
    pthread_mutex_t mutex;
    long progresso_contador; // Usado pelo watchdog
} Recurso;


void inicializar_recurso(Recurso *r, const char *id_str) {
    snprintf(r->id, sizeof(r->id), "%s", id_str);
    pthread_mutex_init(&r->mutex, NULL);
    r->progresso_contador = 0;
}


Recurso ContaA;
Recurso ContaB;
pthread_t thread_t1, thread_t2;
volatile int deadlock_detectado = 0;


void *transferencia_deadlock(void *arg) {
    Recurso **recursos = (Recurso **)arg;
    Recurso *primeiro = recursos[0];
    Recurso *segundo = recursos[1];

    char *nome_thread = (primeiro == &ContaA) ? "T1 (A->B)" : "T2 (B->A)";

    printf("-> %s tentando travar %s\n", nome_thread, primeiro->id);
    pthread_mutex_lock(&primeiro->mutex);
    primeiro->progresso_contador++;
    printf("-> %s travou %s\n", nome_thread, primeiro->id);


    usleep(100000);

    printf("-> %s tentando travar %s\n", nome_thread, segundo->id);


    pthread_mutex_lock(&segundo->mutex);
    segundo->progresso_contador++;


    printf("-> %s travou %s. Transferência concluída.\n", nome_thread, segundo->id);

    pthread_mutex_unlock(&segundo->mutex);
    pthread_mutex_unlock(&primeiro->mutex);

    return NULL;
}


void *watchdog_thread(void *arg) {
    printf("\n*** WATCHDOG INICIADO (Período: %ds) ***\n", TEMPO_INATIVIDADE_S);

    long ultimo_progresso_A = 0;
    long ultimo_progresso_B = 0;
    int contador_inativo = 0; // Contador de checagens sem progresso


    long tempo_checar_us = (TEMPO_INATIVIDADE_S * 1000000) / 2;

    while (!deadlock_detectado) {
        usleep(tempo_checar_us);

        // Verifica o progresso da Conta A
        long prog_A_atual = ContaA.progresso_contador;

        // Verifica o progresso da Conta B
        long prog_B_atual = ContaB.progresso_contador;

        // Se o contador mudou, houve progresso. Reseta o contador de inatividade.
        if (prog_A_atual != ultimo_progresso_A || prog_B_atual != ultimo_progresso_B) {
            contador_inativo = 0;
        } else if (prog_A_atual > 0 || prog_B_atual > 0) {
            // Se não houve mudança *E* algum trabalho já foi iniciado (> 0)
            contador_inativo++;

            // Se a inatividade persistir por tempo suficiente
            if (contador_inativo * tempo_checar_us >= TEMPO_INATIVIDADE_S * 1000000) {
                deadlock_detectado = 1;

                printf("RELATÓRIO DO WATCHDOG - AUSÊNCIA DE PROGRESSO\n");
                printf("Inatividade detectada por mais de %ds em locks mantidos.\n", TEMPO_INATIVIDADE_S);

                // Reportamos os recursos onde o progresso parou.
                fprintf(stderr, "RECURSOS SUSPEITOS DE BLOQUEIO:\n");
                if (prog_A_atual == ultimo_progresso_A && prog_A_atual > 0) {
                    fprintf(stderr, " -> Recurso: %s (Progresso Parado: %ld)\n", ContaA.id, prog_A_atual);
                }
                if (prog_B_atual == ultimo_progresso_B && prog_B_atual > 0) {
                    fprintf(stderr, " -> Recurso: %s (Progresso Parado: %ld)\n", ContaB.id, prog_B_atual);
                }
                fprintf(stderr, "As threads T1 e T2 estão em provável Deadlock.\n");

            }
        }

        ultimo_progresso_A = prog_A_atual;
        ultimo_progresso_B = prog_B_atual;
    }

    printf("\n WATCHDOG ENCERRADO\n");
    return NULL;
}

void *transferencia_corrigida(void *arg) {
    Recurso **recursos = (Recurso **)arg;
    Recurso *de = recursos[0];
    Recurso *para = recursos[1];

    Recurso *primeiro_lock, *segundo_lock;


    if (strcmp(de->id, para->id) < 0) {

        primeiro_lock = de;
        segundo_lock = para;
    } else {

        primeiro_lock = para;
        segundo_lock = de;
    }

    char *nome_thread = (de == &ContaA) ? "T1 (A->B)" : "T2 (B->A)";

    printf("-> %s [CORRIGIDO] tentando travar (1º) %s\n", nome_thread, primeiro_lock->id);
    pthread_mutex_lock(&primeiro_lock->mutex);
    primeiro_lock->progresso_contador++;
    printf("-> %s [CORRIGIDO] travou (1º) %s\n", nome_thread, primeiro_lock->id);

    usleep(100000); // 100ms

    printf("-> %s [CORRIGIDO] tentando travar (2º) %s\n", nome_thread, segundo_lock->id);

    // A thread T2 terá que esperar aqui pelo Lock A, mas T1 irá liberá-lo, evitando o deadlock.
    pthread_mutex_lock(&segundo_lock->mutex);
    segundo_lock->progresso_contador++;

    printf("-> %s [CORRIGIDO] travou (2º) %s. Transferência concluída.\n", nome_thread, segundo_lock->id);

    pthread_mutex_unlock(&segundo_lock->mutex);
    pthread_mutex_unlock(&primeiro_lock->mutex);

    return NULL;
}



void main_deadlock() {
    printf("INICIANDO CENÁRIO DE DEADLOCK (Locks em Ordem Inversa)\n");

    // 1. Inicializa Recursos
    inicializar_recurso(&ContaA, "A1");
    inicializar_recurso(&ContaB, "B2"); // Note que 'B2' é maior que 'A1'

    // 2. Argumentos para as threads
    Recurso *args_t1[] = {&ContaA, &ContaB}; // T1 tenta A1, depois B2
    Recurso *args_t2[] = {&ContaB, &ContaA}; // T2 tenta B2, depois A1 (Ordem inversa)

    // 3. Cria e Inicia Threads
    deadlock_detectado = 0; // Reseta a flag para o watchdog
    pthread_t watchdog;
    pthread_create(&watchdog, NULL, watchdog_thread, NULL);
    pthread_create(&thread_t1, NULL, transferencia_deadlock, args_t1);
    pthread_create(&thread_t2, NULL, transferencia_deadlock, args_t2);

    // 4. Espera pelo watchdog para detectar o problema
    pthread_join(watchdog, NULL);

    printf("FIM DO CENÁRIO DE DEADLOCK\n");
}


void main_corrigido() {
    printf("--- INICIANDO CENÁRIO CORRIGIDO (Ordem Total de Travamento) ---\n");

    // 1. Reinicializa Recursos
    inicializar_recurso(&ContaA, "A1");
    inicializar_recurso(&ContaB, "B2");

    // 2. Argumentos para as threads (sempre passamos a origem e o destino)
    Recurso *args_t1[] = {&ContaA, &ContaB}; // T1: De A1 para B2
    Recurso *args_t2[] = {&ContaB, &ContaA}; // T2: De B2 para A1

    // 3. Cria e Inicia Threads
    pthread_t watchdog;
    pthread_create(&watchdog, NULL, watchdog_thread, NULL); // O watchdog não deve reportar nada

    pthread_create(&thread_t1, NULL, transferencia_corrigida, args_t1);
    pthread_create(&thread_t2, NULL, transferencia_corrigida, args_t2);

    // 4. Espera as threads terminarem (devem terminar rapidamente)
    pthread_join(thread_t1, NULL);
    pthread_join(thread_t2, NULL);

    // 5. Encerra o watchdog
    deadlock_detectado = 1;
    pthread_join(watchdog, NULL);

    printf("--- FIM DO CENÁRIO CORRIGIDO ---\n");
}

int main() {
    main_deadlock();
    main_corrigido();

    // Limpeza dos mutexes
    pthread_mutex_destroy(&ContaA.mutex);
    pthread_mutex_destroy(&ContaB.mutex);

    return 0;
}