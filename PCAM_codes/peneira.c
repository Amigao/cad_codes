#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

int main()
{
    int n;
    printf("Qual o tamanho do vetor? ");
    scanf("%d", &n);

    int *v = (int*) calloc(n + 1, sizeof(int));  // 0 = não marcado (primo), 1 = marcado (composto)
    if (v == NULL) {
        printf("Erro ao alocar memória\n");
        return 1;
    }

    v[0] = v[1] = 1;

    omp_set_nested(1);  // Ativa paralelismo aninhado
    int K = 2;

    while (K * K <= n) {
        int next_K = n + 1;
        // Paraleliza por fatias externas (1º nível)
        #pragma omp parallel reduction(min: next_K)
        {
            int num_threads = omp_get_num_threads();
            int tid = omp_get_thread_num();
            int chunk_size;
            // Determina o tamanho do bloco de trabalho
            chunk_size = (n - K + 1) / num_threads;  // Para os próximos K, começa no K*K

            int ini = K + 1 + tid * chunk_size;
            int fim = (tid == num_threads - 1) ? n : ini + chunk_size - 1;
        
            int local_next_K = n + 1;
            // Paraleliza dentro da fatia (2º nível)
            #pragma omp parallel for reduction(min: local_next_K)
            for (int i = ini; i <= fim; i++) {
                if (i % K == 0 && i >= K * K) {
                    v[i] = 1;
                }
                if (v[i] == 0 
                    && i < local_next_K
                ) {
                    local_next_K = i;
                }
            }
            printf("Thread %d: local_next_K = %d\n", tid, local_next_K);
            next_K = local_next_K;
        }
        
        K = next_K;
        printf("K = %d\n", K);
    }
    
    int contador = 0;
    printf("Primos até %d:\n", n);
    for (int i = 2; i <= n; i++) {
        if (v[i] == 0) {
            contador++;
            printf("%d ", i);
        }
    }
    printf("\nNúmero de primos: %d\n", contador);

    free(v);
    return 0;
}
