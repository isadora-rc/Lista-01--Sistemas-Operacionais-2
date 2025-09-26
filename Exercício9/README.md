# Exercício 09
## Modele uma corrida de revezamento em que K threads representam uma equipe e todas precisam alcançar uma barreira para liberar a próxima “perna” da prova; use pthread_barrier_t (ou implemente uma barreira com mutex/condvar) e registre quantas rodadas são concluídas por minuto sob diferentes tamanhos de equipe.o; particione o arquivo em blocos, faça "map" local em cada thread e "reduce" na principal com exclusão mútua mínima; meça o speedup para P = 1, 2, 4, 8

### Descrição
Implementação de uma simulação de corrida de revezamento usando threads POSIX(pthreads) e a primitiva pthread_barrier_t.
O objetivo é modelar uma equipe de corredores onde todos precisam alcançar a barreira para liberar a próxima "perna" da prova.


**Como funciona?**
1. **Modelo da Equipe:** uma equipe é definida por TAM_EQUIPE (K threads).
2. **Corrida:** simula com o simularPerna().
3. **Sincronização:** todas as threads se encontram na barreira. A barreira suspende todas as threads até que o número exato de corredores chegue.
4. **Conclusaão da Rodada:** apenas quando a última thread chega na barreira que incrementa o contador de rodadas concluídas.

### Compilação e execução
Para rodar a simulação, é necessário ajustar os parâmetros no início do arquivo main.c: *TAM_EQUIPE* e *DURACAO_TESTE_SEGUNDOS*.

``````
gcc -o revezamento main.c 
./revezamento
``````


### Decisões de sincronização
A decisão de sincronização utilizada foi pthread_barrier_t, que sincroniza todas as K threads da equipe. Uma rodada da corrida só é liberada quando todas as threads chegarem ao ponto de encontro (pthread_barrier_wait). Isso garante que a próxima "perna" só comece quando a equipe inteira tiver completado a perna atual.

### Evidências de execução
Realizei teste com K = 2, 4, 8 e 16 corredores.

Resultado 01: K = 2

````
Tamanho da equipe: 2
Rodadas totais concluídas: 421
Tempo real decorrido: 100.00
Rodadas concluídas por minuto: 252.59

````

Resultado 02: K = 4

```
Tamanho da equipe:  4
Rodadas totais concluídas: 387
Tempo real decorrido: 100.00
Rodadas concluídas por minuto: 232.20

```

Resultado 03: K = 8

````
Tamanho da equipe: 8
Rodadas totais concluídas: 361
Tempo real decorrido: 100.00
Rodadas concluídas por minuto: 216.60

````

Resultado 04: K = 16

```
Tamanho da equipe: 16
Rodadas totais concluídas: 348
Tempo real decorrido: 100.00
Rodadas concluídas por minuto: 208.80

```

### Análise dos resultados
À medida que o número de corredores (K) aumenta, a taxa de rodadas concluídas por minuto diminui. Houve sobrecarga de concorrência. 
