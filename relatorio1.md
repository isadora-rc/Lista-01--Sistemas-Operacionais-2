# Relatório Ex1 - Corrida de Cavalos com Threads

## 1. Compilação e Execução

### Para compilar o projeto, utilize o GCC. É necessário linkar a biblioteca Pthreads ("-lpthread").
    gcc ex1.c -o ex1 -lpthread

### Para iniciar a simulação, execute o arquivo compilado:
    ./ex1


## 2. Decisões de Sincronização

### Largada Sincronizada (`pthread_barrier_t`)

-   Garantir que todos os cavalos (threads) comecem a corrida exatamente ao mesmo tempo, sem que um comece antes do outro.
-   Foi utilizada uma `pthread_barrier_t` (`barreira_largada`). A barreira foi inicializada para esperar por `N+1` threads (N cavalos + a thread `main`). Cada thread de cavalo, após ser criada, chama `pthread_barrier_wait()`. A thread `main`, após criar todas as threads, também chama a função. Somente quando todas as `N+1` threads chegam a este ponto é que a barreira é liberada e todas retomam a execução simultaneamente, garantindo uma largada justa.

### Atualização do Placar (`pthread_mutex_t`)

-   O placar (array `posicoes_cavalos`) é uma estrutura de dados compartilhada. Múltiplas threads de cavalos tentam escrever suas novas posições ao mesmo tempo, enquanto a thread `main` tenta ler esses dados para imprimir na tela. Isso pode levar a condições de corrida e "leituras sujas", resultando em um placar visualmente corrompido.
-   Foi utilizado um `pthread_mutex_t` (`mutex_placar`). Antes de qualquer thread modificar o array `posicoes_cavalos` ou a thread `main` ler os dados para imprimir, o mutex é travado (`pthread_mutex_lock`). Após a operação, o mutex é liberado (`pthread_mutex_unlock`), garantindo exclusão mútua e a integridade dos dados do placar.

### Registro do Vencedor e Resolução de Empates (`pthread_mutex_t`)

-   Foi utilizado um segundo mutex, `mutex_vencedor`, para proteger a lógica de verificação e definição do vencedor. Quando um cavalo cruza a linha de chegada, ele trava este mutex. Em seguida, ele executa uma verificação atômica: `if (cavalo_vencedor == -1)`. Se a condição for verdadeira, ele define a si mesmo como o vencedor e atualiza a flag `corrida_acabou` para `true`. Em seguida, libera o mutex. Qualquer outra thread que chegue depois, mesmo que milissegundos depois, encontrará o mutex travado ou, se conseguir travá-lo, verá que `cavalo_vencedor` não é mais `-1` e não fará nada. Isso garante que o primeiro a chegar à seção crítica é o único vencedor.

## 3. Evidências de Execução

**Exemplo de Execução (em andamento):**

[alt text](<Screenshot from 2025-09-25 19-45-30.png>)

**Exemplo de Resultado Final:**
![alt text](<Screenshot from 2025-09-25 19-46-33.png>)

## 4. Análise dos Resultados

-   A barreira garante uma largada justa e sincronizada. O uso do mutex no placar evita inconsistências visuais, resultando em uma animação fluida. Mais importante, a seção crítica para determinar o vencedor, protegida por um mutex dedicado, previne condições de corrida, assegurando que um único e determinístico vencedor seja sempre declarado, mesmo em cenários de chegadas muito próximas. Os testes com diferentes números de threads mostraram que a lógica de sincronização é robusta e funciona como o esperado.