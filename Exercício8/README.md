# Exercício 08
## Estenda o exercício 2 para simular rajadas de produção (bursts) e períodos de ociosidade; implemente backpressure fazendo produtores aguardarem quando a taxa de consumo cair, e registre a ocupação do buffer ao longo do tempo para analisar estabilidade.

### Descrição
Este programa implementa o problema clássico Produtor-Consumidor usando threads, semáforos e mutexes. Um buffer circular de tamanho limitado armazena itens produzidos em rajadas, enquanto consumidores os retiram, medindo o tempo de espera e garantindo sincronização.

### Como funciona?

O código simula um sistema de fila de mensagens (buffer circular) onde múltiplos produtores geram itens e múltiplos consumidores os processam.

As principais características desta implementação são:

1. **Buffer Limitado:** Gerenciado como uma fila circular de tamanho N.
2. **Sincronização:** Uso de **semáforos** para controlar a capacidade do buffer e **mutexes** para proteger o acesso às variáveis compartilhadas (`in`, `out`, `produced`, `consumed`).
3. **Simulação de Carga:** Os produtores operam em ciclos de **rajadas** (produção rápida de `BURST_SIZE` itens) seguidas por longos **períodos de ociosidade** (simulando variabilidade na taxa de produção).
4. **Contrapressão (Backpressure):** Um mecanismo implementado usando **mutexes e variáveis de condição (`bp_mutex`, `bp_cond`)** que força os produtores a pausarem se a ocupação do buffer exceder um limite (`BACKPRESSURE_THRESHOLD`, 75%).
5. **Análise de Estabilidade:** A ocupação do buffer é exibida ao longo do tempo, permitindo verificar como o sistema (e o *backpressure*) reage às rajadas de carga.

Cada produtor executa as seguintes etapas em loop:

1. **Ociosidade:** `usleep` por um tempo aleatório e relativamente longo para simular ausência de trabalho.
2. **Backpressure:** Verifica a ocupação do buffer. Se estiver acima de 75%, o produtor bloqueia (`pthread_cond_wait`) até que um consumidor o sinalize.
3. **Rajada (Burst):** Produz `BURST_SIZE` itens rapidamente, com um pequeno `usleep` entre eles.
4. **Sincronização:** Usa `sem_wait(&empty)` para garantir que haja espaço.
5. **Registro:** Atualiza métricas e estatísticas de ocupação do buffer.

Cada consumidor roda em loop infinito até todos os produtores terminarem:

1. **Sincronização:** Usa `sem_wait(&full)` para garantir que haja itens e `sem_post(&empty)` após remover um item.
2. **Medição:** Calcula o tempo de espera (latência) de cada item no buffer (`ts_cons - ts_prod`).
3. **Sinalização do Backpressure:** Se a ocupação do buffer cair **abaixo do limite**, o consumidor usa `pthread_cond_broadcast(&bp_cond)` para acordar produtores bloqueados.
4. **Saída Limpa:** Quando `producers_finished = 1` e todos os itens foram consumidos, o loop é encerrado.

Fluxo do main

1. Cria threads de produtores e consumidores.
2. Aguarda todos os produtores finalizarem.
3. Define `producers_finished = 1` e sinaliza consumidores (`sem_post(&full)`) para que acordem e encerrem.
4. Aguarda o término dos consumidores.
5. Calcula e exibe métricas:
    - **Tamanho do Buffer**
    - **Produtores e Consumidores ativos**
    - **Itens produzidos e consumidos**
    - **Tempo total de execução**
    - **Throughput (itens/s)**
    - **Tempo médio de espera no buffer**


### Compilação e execução
```
gcc main.c -o main -pthread
./main <tamanho_buffer> <qtd_produtores> <qtd_consumidores>
```

### Evidências de execução

````
Tamanho do Buffer: 100
Produtores: 2, Consumidores: 2
Itens produzidos: 20000, Itens consumidos: 20000
Tempo Total: 10.2955 s
Throughput: 1942.60 itens/s
Tempo Medio de Espera (no buffer): 0.007648 s
````

### Análise dos resultados
O sistema conseguiu produzir e consumir todos os itens corretamente, sem perdas, mostrando consistência do buffer.
O throughput (~1942 itens/s) é relativamente baixo, sugerindo que o gargalo está na sincronização (mutex/semaforos) e não na produção/consumo em si.
O tempo médio de espera no buffer (~7,6 ms) indica que houve períodos de fila, mas o consumo conseguiu acompanhar a produção de forma estável.

