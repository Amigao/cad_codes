#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

int main()
{
    int n = 10000;
    int *v = (int*) calloc(n + 1, sizeof(int));
    if (v == NULL) {
        printf("Erro ao alocar memória\n");
        return 1;
    }

    v[0] = v[1] = 1;
    int K = 2;

    double inicio = omp_get_wtime(); //  começa a contagem do tempo

    while (K * K <= n) {
        int next_K = n + 1;

        #pragma omp parallel for reduction(min: next_K)
        for (int i = K + 1; i < n; i++) {
            if (i % K == 0 && i >= K * K) {
                v[i] = 1;
            }
            if (v[i] == 0 && i < next_K) {
                next_K = i;
            }
        }
        K = next_K;
    }

    double fim = omp_get_wtime(); //  fim da contagem

    int contador = 0;
    for (int i = 2; i <= n; i++) {
        if (v[i] == 0) {
            contador++;
        }
    }

    printf("Número de primos: %d\n", contador);
    printf("Tempo do Algoritmo 2: %lf segundos\n", fim - inicio); 

    free(v);
    return 0;
}
