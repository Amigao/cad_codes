#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 9500  // ordem da matriz
#define P 16    // número de tarefas (deve ser quadrado perfeito)

void multiplica_bloco(float **A, float *x, float *y) {
    int sqrt_P = 4;  // pois P = 16 => √P = 4
    int block_size = N / sqrt_P;

    #pragma omp parallel for collapse(2)
    for (int bi = 0; bi < sqrt_P; bi++) {
        for (int bj = 0; bj < sqrt_P; bj++) {
            int row_start = bi * block_size;
            int row_end   = (bi == sqrt_P - 1) ? N : row_start + block_size;

            int col_start = bj * block_size;
            int col_end   = (bj == sqrt_P - 1) ? N : col_start + block_size;

            for (int i = row_start; i < row_end; i++) {
                float acc = 0.0f;
                #pragma omp simd reduction(+:acc)
                for (int j = col_start; j < col_end; j++) {
                    acc += A[i][j] * x[j];
                }
                #pragma omp atomic
                y[i] += acc;
            }
        }
    }
}

int main() {
    // Aloca memória dinamicamente para a matriz A
    float **A = malloc(N * sizeof(float *));
    for (int i = 0; i < N; i++) {
        A[i] = malloc(N * sizeof(float));
    }

    float *x = malloc(N * sizeof(float));
    float *y = calloc(N, sizeof(float));  // inicia com 0

    if (!A || !x || !y) {
        printf("Erro ao alocar memória\n");
        return 1;
    }

    // Inicializa matriz A e vetor x
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = (i + j) % 10 / 2;
            x[j] = j % 10 / 2;
        }
    }

    double start_parallel = omp_get_wtime();
    multiplica_bloco(A, x, y);
    double end_parallel = omp_get_wtime();

    printf("\nResultado y (paralelo):\n");
    for (int i = 0; i < 8; i++) {  // Mostra só os primeiros 8 valores
        printf("%.1f ", y[i]);
    }
    printf("\n");

    // Multiplicação sequencial
    double start_seq = omp_get_wtime();
    for (int i = 0; i < N; i++) {
        float acc = 0.0f;
        for (int j = 0; j < N; j++) {
            acc += A[i][j] * x[j];
        }
        y[i] = acc;
    }
    double end_seq = omp_get_wtime();

    printf("Resultado y (sequencial):\n");
    for (int i = 0; i < 8; i++) {
        printf("%.1f ", y[i]);
    }
    printf("\n");

    printf("Tempo paralelo:   %.6f segundos\n", end_parallel - start_parallel);
    printf("Tempo sequencial: %.6f segundos\n", end_seq - start_seq);

    // Libera memória
    for (int i = 0; i < N; i++) free(A[i]);
    free(A);
    free(x);
    free(y);

    return 0;
}

// Resultado y (paralelo):
// 57000.0 47500.0 38000.0 33250.0 28500.0 28500.0 28500.0 33250.0 
// Resultado y (sequencial):
// 57000.0 47500.0 38000.0 33250.0 28500.0 28500.0 28500.0 33250.0
// Tempo paralelo:   0.100636 segundos
// Tempo sequencial: 0.375931 segundos