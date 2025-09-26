# Exercício 07 

## Estude e implemente o algoritmo dos filósofos com garfos representados por mutex e proponha duas soluções:
## - Ordem global de aquisição para evitar deadlock;
## - Semafóro limitando a quatro filósofos simultâneos; 
## Colete métricas por filósofo(número de refeições, maior espera) e ajuste a lógica para mitigar starvation. 

### Descrição 
**Solução A - Ordem global de aquisição**
<br>*Estratégia:* Sempre adquirir os garfos em ordem crescente de ID, eliminando dependências circulares.
<br>*Como funciona:*
+ Filósofo determina qual garfo tem menor ID.
+ Sempre pega primeiro o garfo de menor ID, depois o maior.
+ Quebra o ciclo de dependência que causa deadlock.

**Solução B - Semafóro limitado a quatro filósofos simultâneos**
<br>*Estratégia:* Limita a 4 filósofos simultâneos no jantar usando semáforos.
<br>*Como funciona:*
+ Semáforo permite no máximo 4 filósofos tentando comer simultaneamente.
+ Com apenas 4 filósofos competindo por 5 garfos, sempre há garfos disponíveis.
+ Evita deadlock para garantir recursos suficientes.


### Compilação e execução 
Para compilar e executar a primeira solução, faça:
```
gcc solucaoA.c -o solucaoA.o
./solucaoA.o
```

Para compilar e executar a segunda solução, faça:
```
gcc solucaoB.c -o solucaoB.o
./solucaoB.o
```


### Decisões de sincronização
**Solução A** = na solução A, foi utilizado o Mutex no garfo. A lógica utilizada é que o filósofo deve identificar o menor e maior indíces de seus garfos.
**Solução B** = na solução B, foi utilizado o Semáforo na sala de jantar e o Mutex no garfo.


### Evidências de execução
Foi definido um tempo de simulação (100 s)
** Saída da solução A** 
```
Filósofo 0: 8 refeições 
Tempo médio: 0.26s
Tempo máximo de espera: 3.69s

Filósofo 1: 7 refeições 
Tempo médio: 0.25s
Tempo máximo de espera: 4.57s

Filósofo 2: 8 refeições 
Tempo médio: 0.39s
Tempo máximo de espera: 4.28s

Filósofo 3: 8 refeições 
Tempo médio: 0.29s
Tempo máximo de espera: 4.77s

Filósofo 4: 7 refeições 
Tempo médio: 0.26s
Tempo máximo de espera: 9.37s

Total de refeições: 38
Média de refeições: 7.0

```



** Saída da solução B** 
```
Filósofo 0: 4 refeições 
Tempo médio: 3.97s
Tempo máximo de espera: 20.55s
 
Filósofo 1: 4 refeições 
Tempo médio: 4.39s
Tempo máximo de espera: 17.55s
 
Filósofo 2: 4 refeições 
Tempo médio: 3.90s
Tempo máximo de espera: 16.37s
 
Filósofo 3: 5 refeições 
Tempo médio: 3.26s
Tempo máximo de espera: 17.03s

Filósofo 4: 5 refeições 
Tempo médio: 3.36s
Tempo máximo de espera: 16.79s
 
Total de refeições: 22
Média de refeições: 4.0
 
```


### Análise de resultados
A Solução A é mais eficiente e justa, uma vez que o tempo de espera entre os filósofos não é alto e eles comem mais vezes. Enquanto que a solução B é lenta e ineficiente. O tempo de espera é muito alto, o que significa que os filósofos passam a maior parte do tempo bloqueados, esperando pelos garfos.
