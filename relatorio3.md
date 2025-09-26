# Relatório Ex03 - Simulação de Transferências Bancárias com Threads e Proteção de Concorrência

## 1. Compilação e Execução

### Para compilar o programa:
    gcc ex3.c -o ex3 -pthread

### Para iniciar a simulação, execute o arquivo compilado:
    ./ex3


## 2. Decisões de Sincronização

### Mutex (trava mútua):

-   Foi utilizado para garantir que apenas uma thread por vez altere o saldo de duas contas. Sem isso, duas threads poderiam mexer no mesmo valor simultaneamente, causando condições de corrida.

### Ordenação por ID:

-   Antes de travar as duas contas envolvidas na transferência, os IDs são comparados e ordenados. Assim todas as threads seguem a mesma ordem de travamento, evitando deadlock.


## 3. Evidências de Execução

### Exemplo de Execução:

[print do terminal](<Screenshot from 2025-09-26 15-32-58.png>)


## 4. Análise dos Resultados

### Cenário sem travas:
-   Houve perda de consistência no sistema. Como várias threads acessam e alteram a mesma memória ao mesmo tempo, a soma total se corrompe. Esse é o exemplo de condição de corrida.

### Cenário com travas:
-   A consistência foi mantida em todas as execuções. Apesar de existir sobrecarga de sincronização (as threads precisam esperar umas pelas outras), o resultado final é confiável. Isso mostra o papel essencial dos mutexes na programação concorrente.