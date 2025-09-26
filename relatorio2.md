# Relatório Ex02 - Buffer Circular com Produtores e Consumidores

## 1. Compilação e Execução

### Para compilar o programa é necessário -lrt para o uso de clock_gettime com CLOCK_MONOTONIC:
    gcc ex2.c -o ex2 -pthread -lrt

### Para iniciar a simulação, execute o arquivo compilado:
    ./ex2 <tamanho_buffer> <num_produtores> <num_consumidores> <itens_por_produtor>

    Exemplo: ./ex2 5 2 2
        5: tamanho do buffer
        2: produtores
        2: consumidores


## 2. Decisões de Sincronização

### Mutex (pthread_mutex_t)
-   Usado para proteger as variáveis compartilhadas (in, out, produced, consumed, total_wait). Garante que apenas uma thread por vez modifique o estado crítico.

### Semáforos (sem_t)

-   empty: controla quantas posições livres há no buffer.
-   full: controla quantos itens disponíveis há no buffer.
Essa combinação elimina espera ativa (busy-wait), já que threads bloqueiam quando não podem prosseguir.

### Estratégia de término

-   Um contador global (producers_finished) sinaliza quando todos os produtores encerraram. Consumidores continuam retirando até o buffer esvaziar e então encerram.


## 3. Evidências de Execução

### Exemplo de Execução:

[print do terminal](<Screenshot from 2025-09-25 22-30-38-1.png>)


## 4. Análise dos Resultados

### Efeito do tamanho do buffer:
-   Quando o buffer é pequeno (N=5), há maior chance de produtores bloquearem esperando espaço ou consumidores bloquearem esperando itens. Isso reduz o throughput e aumenta o tempo médio de espera.

### Com buffers maiores (N=50 e N=500):
-   O throughput aumenta significativamente.
-   O tempo médio de espera diminui, pois há menos bloqueios e os itens fluem mais livremente.

### Conclusão:
-   Buffers muito pequenos limitam a taxa de produção/consumo.
-   Buffers maiores oferecem mais flexibilidade, diminuindo bloqueios e resultando em melhor performance.