# Relatório Ex04 - Pipeline com Threads: captura, processamento e gravação

## 1. Compilação e Execução

### Para compilar o programa:
    gcc ex4.c -o ex4 -pthread

### Para iniciar a simulação, execute o arquivo compilado:
    ./ex4


## 2. Decisões de Sincronização

### Mutex

-   Garante acesso exclusivo à fila quando uma thread adiciona ou remove um item. Evita condições de corrida, garantindo que duas threads não modifiquem a mesma fila simultaneamente.

### Variáveis de condição (pthread_cond_t)

-   nao_vazia: sinaliza que a fila contém itens e threads consumidoras podem desenfileirar.
-   nao_cheia: sinaliza que há espaço na fila e threads produtoras podem enfileirar.

### Poison Pill

-   Valor enviado pelas threads produtoras quando terminam, sinalizando para as threads consumidoras que não haverá mais itens. Evita deadlocks ou espera infinita.


## 3. Evidências de Execução

### Exemplo de Execução:

[print do terminal](<Screenshot from 2025-09-26 15-51-11.png>)


## 4. Análise dos Resultados

### Fluxo correto de dados:

-   Cada item gerado na captura é processado e gravado, confirmando que as filas limitadas e mutexes funcionam conforme esperado.

### Evita deadlocks:

-   Não há espera infinita, pois todas as filas possuem sinalização apropriada (pthread_cond_signal) e as threads encerram corretamente com a pilula de veneno.

### Desempenho controlado:

-   O uso de filas limitadas (TAMANHO_FILA = 5) permite que threads produtoras e consumidoras sejam desacopladas, evitando sobrecarga.

### Encerramento limpo:

-   O protocolo de poison pill garante que, após processar todos os itens, as threads se encerrem sem travamentos.