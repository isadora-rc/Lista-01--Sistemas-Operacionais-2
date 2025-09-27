# Exercício 10 
## Programe um cenário com múltiplos recursos e threads que, propositalmente, podem se bloquear por adquirir locks em ordens distintas; crie uma thread“watchdog” que detecta ausência de progresso por T segundos e emite um relatório dos recursos/threads suspeitos; em seguida, corrija adotando uma ordem total de travamento e compare os comportamentos.

### Descrição 
Este projeto demonstra, em C utilizando POSIX Threads (pthreads), como a aquisição inconsistente de locks pode levar a um deadlock e como uma thread watchdog pode detectar essa ausência de progresso. Em seguida, a solução clássica, a Ordem Total de Travamento, é aplicada para eliminar o problema.

### Como funciona?

O cenário simula a transferência de fundos entre duas contas bancárias, onde cada conta é protegida por um pthread_mutex_t.
+ **Recursos:** struct que contém um pthread_mutex_t e um progresso_contador.
+ **Threads principais:** Thread 1(T1) - tenta transferir de A para B Thread 2(T2) - tenta transferir de B para A.
+ **Thread de monitoramente:** Watchdog - Uma thread de cão de guarda que periodicamente verifica o progresso_contador de cada recurso. Se o contador de um recurso não mudar por T segundos, ele reporta um possível deadlock.

**Cenário 1: Deadlock Proposital (Comportamento Inconsistente)**
Objetivo: Causar um bloqueio mútuo (deadlock).

*Mecanismo do Deadlock*

No cenário de deadlock, as threads adquirem os locks em ordens inversas:
T1: Adquire Lock de A e espera pelo Lock de B.
T2: Adquire Lock de B e espera pelo Lock de A.
Como cada thread possui o recurso que a outra precisa, o ciclo de espera se forma e nenhuma avança.



*Ação do Watchdog*

O Watchdog detecta o problema:
O progresso_contador das contas para de ser incrementado, mas mantém um valor maior que zero (indicando que o trabalho foi iniciado).
Após o tempo limite (T segundos, definido como TEMPO_INATIVIDADE_S), o Watchdog imprime um relatório no stderr (saída de erro) indicando a ausência de progresso e a suspeita de deadlock, forçando o fim do primeiro cenário.

**Cenário 2: Correção (Ordem Total de Travamento)**
Objetivo: Eliminar a condição de espera circular e resolver o deadlock.

*Princípio da Correção*

A correção adota uma ordem total (ou hierarquia) para a aquisição de locks:
"Sempre que uma thread precisar de múltiplos locks, ela deve adquiri-los na mesma ordem consistente, baseada em um identificador único (e.g., o ID da conta)."
No código, isso é implementado usando a função strcmp() nos IDs dos recursos. Ambas as threads agora sempre tentam travar o recurso de menor ID primeiro.

*Comportamento Corrigido*

Ambas as threads tentam travar o recurso com o menor ID (por exemplo, A).
Uma thread (e.g., T1) trava A e prossegue. T2 bloqueia em A, aguardando.
T1 trava B, conclui o trabalho e libera A e B.
T2 obtém o lock em A, prossegue e conclui seu trabalho.

Resultado: As transferências são concluídas com sucesso. O Watchdog não detecta ausência de progresso e o programa encerra normalmente.

### Compilação e execução
````
gcc deadlock.c -o deadlock -pthread
./deadlock
````

### Evidências de execução
````
INICIANDO CENÁRIO DE DEADLOCK (Locks em Ordem Inversa)

 WATCHDOG INICIADO (Período: 3s) 
-> T1 (A->B) tentando travar A1
-> T2 (B->A) tentando travar B2
-> T2 (B->A) travou B2
-> T1 (A->B) travou A1
-> T1 (A->B) tentando travar B2
-> T2 (B->A) tentando travar A1
RELATÓRIO DO WATCHDOG - AUSÊNCIA DE PROGRESSO
Inatividade detectada por mais de 3s em locks mantidos.
RECURSOS SUSPEITOS DE BLOQUEIO:
 -> Recurso: A1 (Progresso Parado: 1)
 -> Recurso: B2 (Progresso Parado: 1)
As threads T1 e T2 estão em provável Deadlock.

 WATCHDOG ENCERRADO
FIM DO CENÁRIO DE DEADLOCK


INICIANDO CENÁRIO CORRIGIDO (Ordem Total de Travamento) 
WATCHDOG INICIADO (Período: 3s) 
WATCHDOG ENCERRADO
-> T1 (A->B) [CORRIGIDO] tentando travar (1º) A1
-> T1 (A->B) [CORRIGIDO] travou (1º) A1
-> T2 (B->A) [CORRIGIDO] tentando travar (1º) A1
-> T1 (A->B) [CORRIGIDO] tentando travar (2º) B2
-> T1 (A->B) [CORRIGIDO] travou (2º) B2. Transferência concluída.
-> T2 (B->A) travou A1. Transferência concluída.
-> T2 (B->A) [CORRIGIDO] travou (1º) A1
-> T2 (B->A) [CORRIGIDO] tentando travar (2º) B2
-> T2 (B->A) [CORRIGIDO] travou (2º) B2. Transferência concluída.
FIM DO CENÁRIO CORRIGIDO 
````

### Análise de resultados
A imposição de uma ordem total no travamento de recursos é um método eficaz para prevenir deadlocks, contrastando com o travamento inconsistente que resultou em um bloqueio permanente.
