# Exercício 06

## Leia um arquivo grande de inteiros e calcule (a) soma total (b) histograma de frequências usando P threads em paralelo; particione o arquivo em blocos, faça "map" local em cada thread e "reduce" na principal com exclusão mútua mínima; meça o speedup para P = 1, 2, 4, 8
### Descrição
Para esse exercício, foi criado 2 códigos-fontes: geradorArquivo.c e
main.c. 

Em geradorArquivo.c, é criado um arquivo que contém os números inteiros para serem processados pelo main.c.

Em main.c, é feito o processamento de histograma de frequências e a soma total pelas threads. 


### Compilação e execução
É necessário criar um arquivo primeiro, logo em geradorArquivo.c, faça:

``````
gcc geradorArquivo.c -o geradorArquivo.o   
./geradorArquivo.o <nome_do_arquivo>.bin <quantidade_inteiros>  
``````

Depois, em main.c, faça:

````
gcc main.c -o main.o   
./main.o <nome_do_arquivo>.bin    
````

### Decisões de sincronização
A decisão de sincronização utilizada foi o Mutex (exclusão mútua). O mutex foi usado para proteger o acesso a duas variáveis globais que são compartilhadas por todas as threads:
*somaTotal* e *histogramaTotal*.

A solução adota o conceito de **exclusão mútua mínima**, o que significa que o mutex é aplicado apenas na menor porção de código possível. Existem duas fases:
+ **Fase Map**: As threads leem o arquivo e calculam a soma e o histograma localmente sem usar o mutex.
+ **Fase Reduce**: O mutex é travado apenas no momento em que a thread soma seus resultados locais às variáveis globais, e em seguida, é liberado imediatamente.  

### Evidências de execução 
Utilizei um arquivo de 1000000 inteiros para fazer o cálculo. 

Resultado 01: Com 1 Thread somente (sequencial)

````
Tempo de execução:  0.310768 segundos.
Soma total: 4994418219

````

Resultado 02: Com 2 threads.
```
Tempo de execução: 0.197073 segundos.
Soma total: 4994418219

```

Resultado 03: Com 04 threads.
```
Tempo de execução: 0.150924 segundos.
Soma total: 4994418219

```

Resultado 04: Com 08 threads.
```
Tempo de execução: 0.084611 segundos.
Soma total: 4994418219

```

### Análise dos resultados

A partir das evidências de execução, é perceptível analisar que quanto maior o número de threads para o processamento, mais rápido o programa finaliza.

Speedups calculados: 
```
Speedup2 = 0.310768 / 0.197073  1.576918
Speedup4 =  0.310768 / 0.150924 = 2.059102
Speedup8 = 0.310768 / 0.084611 = 3.672903

```

