# Relatório Ex05 - Pool de Threads com Fila Concorrente

## 1. Compilação e Execução

### Para compilar o programa:
    gcc ex5.c -o ex5 -pthread

### Para iniciar a simulação, execute o arquivo compilado:
    ./ex5


## 2. Decisões de Sincronização

### Mutex (pthread_mutex_t):

-   Protege o acesso à fila, garantindo que apenas uma thread por vez possa enfileirar ou desenfileirar uma tarefa. Evita condições de corrida.

### Variável de condição (pthread_cond_t):

-   Utilizada para que threads workers esperem quando a fila está vazia.
-   pthread_cond_wait: faz a thread aguardar por novas tarefas.
-   pthread_cond_signal: acorda uma thread quando uma nova tarefa é adicionada.
-   pthread_cond_broadcast: é usado no fechamento da fila para acordar todas as threads e permitir que terminem.


## 3. Evidências de Execução

### Exemplo de Execução:

[print do terminal](<Screenshot from 2025-09-26 16-50-38.png>)

## 4. Análise dos Resultados
### O programa demonstrou execução concorrente eficiente com múltiplas threads.

### A utilização de mutex e variáveis de condição garantiu que:

-   Nenhuma tarefa fosse perdida.
-   Threads não entrassem em condições de corrida.
-   Threads bloqueadas acordassem corretamente quando novas tarefas fossem adicionadas ou quando a fila fosse fechada.

### As tarefas CPU-bound (cálculo de Fibonacci) foram distribuídas entre as threads, mostrando paralelismo real.

### O fechamento da fila e o pthread_join garantiram que todos os recursos fossem liberados corretamente, sem vazamento de memória.