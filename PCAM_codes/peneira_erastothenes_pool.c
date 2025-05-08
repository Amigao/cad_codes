#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

#define N 10000  // Pode ser alterado dinamicamente também

int NUM_THREADS;     // Detectado em tempo de execução
int BLOCO;           // Tamanho de cada bloco
int NUM_BLOCOS;      // Quantidade total de blocos

bool* primos;             // Vetor de primalidade
int* blocos_ocupados;     // Bitset com flags de ocupação

void processar_bloco(int bloco_id, int k, int tid) {
    int inicio = bloco_id * BLOCO;
    int fim = inicio + BLOCO;
    if (fim > N) fim = N;

    // printf("Thread %d processando bloco %d (de %d até %d) com k=%d\n", tid, bloco_id, inicio, fim - 1, k);

    for (int i = inicio; i < fim; i++) {
        if (i > k && i % k == 0) {
            primos[i] = false;
        }
    }
}

int buscar_bloco_disponivel() {
    int bloco = -1;
    for (int i = 0; i < NUM_BLOCOS; i++) {
        int ocupado;

        #pragma omp critical
        {
            ocupado = blocos_ocupados[i];
            if (ocupado == 0) {
                blocos_ocupados[i] = 1;
                bloco = i;
            }
        }

        if (bloco != -1) break;
    }
    return bloco;
}

void peneira_parallel() {
    // Descobre número de threads disponíveis
    NUM_THREADS = omp_get_max_threads();
    BLOCO = (N + NUM_THREADS - 1) / NUM_THREADS;
    NUM_BLOCOS = (N + BLOCO - 1) / BLOCO;

    // printf("Número de threads: %d\n", NUM_THREADS);
    // printf("Tamanho do vetor: %d, número de blocos: %d, tamanho do bloco: %d\n", N, NUM_BLOCOS, BLOCO);

    // Aloca vetores
    primos = malloc(N * sizeof(bool));
    blocos_ocupados = calloc(NUM_BLOCOS, sizeof(int));
    for (int i = 0; i < NUM_THREADS && i < NUM_BLOCOS; i++) {
        blocos_ocupados[i] = 1;  // já marcar blocos garantidos
    }

    // Inicializa vetor de primos
    for (int i = 0; i < N; i++) primos[i] = true;
    primos[0] = primos[1] = false;

    int* k_local = malloc(NUM_THREADS * sizeof(int));
    int* k_recebido = calloc(NUM_THREADS, sizeof(int));


    for (int k = 2; k * k < N; k++) {
        if (!primos[k]) continue;

        // printf("\n==== Começando iteração com k = %d ====\n", k);

        for (int i = 0; i < NUM_BLOCOS; i++) blocos_ocupados[i] = 0;
        for (int i = 0; i < NUM_THREADS; i++) k_recebido[i] = 0;

        k_local[0] = k;  // Thread 0 começa com o k
        k_recebido[0] = 1;

        #pragma omp parallel num_threads(NUM_THREADS)
        {
            int tid = omp_get_thread_num();

            // Comunicação binária: espera o pai liberar o valor de k
            if (tid != 0) {
                int pai = (tid - 1) / 2;
                while (!k_recebido[pai]) {
                    #pragma omp flush(k_recebido)
                }
                k_local[tid] = k_local[pai];  // herda o k
                k_recebido[tid] = 1;
                #pragma omp flush(k_recebido)
                // printf("Thread %d recebeu k = %d do pai %d\n", tid, k_local[tid], pai);
            } else {
                // printf("Thread 0 iniciou com k = %d\n", k_local[0]);
            }

            int k_valor = k_local[tid];

            // Cada thread processa o bloco correspondente ao seu tid (já marcado previamente)
            if (tid < NUM_BLOCOS) {
                processar_bloco(tid, k_valor, tid);
            }

            // Agora continua disputando dinamicamente os blocos restantes
            while (1) {
                int bloco = buscar_bloco_disponivel();
                if (bloco == -1) break;
                processar_bloco(bloco, k_valor, tid);
            }

            // printf("Thread %d terminou a iteração com k = %d\n", tid, k_valor);
        }
    }

    int contador = 0;
    printf("\nPrimos encontrados:\n");
    for (int i = 2; i < N; i++) {
        if (primos[i]) {
            // printf("%d ", i);
            contador++;
        }
    }
    printf("\nNumero de primos: %d\n", contador);

    free(primos);
    free(blocos_ocupados);
    free(k_local);
    free(k_recebido);
}

int main() {    
    double start, end;

    // Tempo do algoritmo 1
    start = omp_get_wtime();
    peneira_parallel();
    end = omp_get_wtime();
    printf("Tempo do Algoritmo 1: %.6f segundos\n", end - start);
    return 0;
}
