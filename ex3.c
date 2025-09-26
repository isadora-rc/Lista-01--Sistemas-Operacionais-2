#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>

#define NUM_CONTAS 100
#define NUM_THREADS 8
#define TRANSFERENCIAS_POR_THREAD 10000

typedef struct {
    int id;
    double saldo;
    pthread_mutex_t trava;
} Conta;

typedef struct {
    Conta* contas;
    int transferencias_por_thread;
    int num_contas;
} DadosThread;

Conta contas_travadas[NUM_CONTAS];
Conta contas_sem_trava[NUM_CONTAS];

//transferencia com travas (cenario correto)
void* transferencia_com_trava(void* arg) {
    DadosThread* dados = (DadosThread*)arg;
    for (int i = 0; i < dados->transferencias_por_thread; i++) {
        int origem = rand() % dados->num_contas;
        int destino = rand() % dados->num_contas;
        double valor = (double)(rand() % 100) + 1.0;

        if (origem == destino) {
            continue;
        }

        //evitar deadlock
        Conta* c1 = &dados->contas[origem];
        Conta* c2 = &dados->contas[destino];

        if (c1->id > c2->id) {
            Conta* temp = c1;
            c1 = c2;
            c2 = temp;
        }

        pthread_mutex_lock(&c1->trava);
        pthread_mutex_lock(&c2->trava);

        if (c1->saldo >= valor) {
            c1->saldo -= valor;
            c2->saldo += valor;
        }

        pthread_mutex_unlock(&c2->trava);
        pthread_mutex_unlock(&c1->trava);
    }
    return NULL;
}

//transferencia sem travas (cenario incorreto)
void* transferencia_sem_trava(void* arg) {
    DadosThread* dados = (DadosThread*)arg;
    for (int i = 0; i < dados->transferencias_por_thread; i++) {
        int origem = rand() % dados->num_contas;
        int destino = rand() % dados->num_contas;
        double valor = (double)(rand() % 100) + 1.0;

        if (origem == destino) {
            continue;
        }

        if (dados->contas[origem].saldo >= valor) {
            dados->contas[origem].saldo -= valor;
            dados->contas[destino].saldo += valor;
        }
    }
    return NULL;
}

void inicializa_contas(Conta contas[], int num_contas) {
    for (int i = 0; i < num_contas; i++) {
        contas[i].id = i;
        contas[i].saldo = 1000.0;
        pthread_mutex_init(&contas[i].trava, NULL);
    }
}

void limpa_contas(Conta contas[], int num_contas) {
    for (int i = 0; i < num_contas; i++) {
        pthread_mutex_destroy(&contas[i].trava);
    }
}

double soma_total(Conta contas[], int num_contas) {
    double total = 0.0;
    for (int i = 0; i < num_contas; i++) {
        total += contas[i].saldo;
    }
    return total;
}


int main() {
    srand(time(NULL));

    pthread_t threads[NUM_THREADS];
    DadosThread dados_trava, dados_sem_trava;
    
    //com Travas
    printf("Cenário com Travas (correto):\n");
    inicializa_contas(contas_travadas, NUM_CONTAS);
    
    double soma_inicial_trava = soma_total(contas_travadas, NUM_CONTAS);
    printf("Soma inicial= %.2f\n", soma_inicial_trava);

    dados_trava.contas = contas_travadas;
    dados_trava.transferencias_por_thread = TRANSFERENCIAS_POR_THREAD;
    dados_trava.num_contas = NUM_CONTAS;

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, transferencia_com_trava, &dados_trava);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    double soma_final_trava = soma_total(contas_travadas, NUM_CONTAS);
    printf("Soma final= %.2f\n", soma_final_trava);

    assert(soma_final_trava == soma_inicial_trava);
    printf("(%.2f == %.2f)\n\n", soma_final_trava, soma_inicial_trava);

    limpa_contas(contas_travadas, NUM_CONTAS);

    //sem Travas
    printf("Cenário sem Travas (incorreto):\n");
    inicializa_contas(contas_sem_trava, NUM_CONTAS);
    
    double soma_inicial_sem = soma_total(contas_sem_trava, NUM_CONTAS);
    printf("Soma inicial= %.2f\n", soma_inicial_sem);

    dados_sem_trava.contas = contas_sem_trava;
    dados_sem_trava.transferencias_por_thread = TRANSFERENCIAS_POR_THREAD;
    dados_sem_trava.num_contas = NUM_CONTAS;

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, transferencia_sem_trava, &dados_sem_trava);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    double soma_final_sem = soma_total(contas_sem_trava, NUM_CONTAS);
    printf("Soma final= %.2f\n", soma_final_sem);

    if (soma_final_sem == soma_inicial_sem) {
        printf("A soma foi mantida mesmo sem travas.\n");
    } else {
        printf("Condicao de corrida... (%.2f != %.2f)\n", soma_final_sem, soma_inicial_sem);
    }
    
    limpa_contas(contas_sem_trava, NUM_CONTAS);

    return 0;
}
