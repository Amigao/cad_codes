#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h> 

#define MAXIMO_CARACTERES 96 // total ascii fornecido
#define TAMANHO_MAX_LINHA 1000   //tamanho maximo linha
#define MAX_LINHAS 1000

int todas_freq[MAX_LINHAS][MAXIMO_CARACTERES];  // Frequências por linha
int todas_ascii[MAX_LINHAS][MAXIMO_CARACTERES]; // ASCII por linha
char linhas[MAX_LINHAS][TAMANHO_MAX_LINHA];
int ascii_count[MAX_LINHAS];                    // Quantidade de letras distintas por linha
int total_linhas = 0;


void frequencia(char *linha, int *freq) {
    size_t tamanho_string = strlen(linha);  
    printf("lido: '%s'\n", linha); 
    #pragma omp parallel for
    for (int i = 0; i < tamanho_string; i++) {   // uma tarefa pra cada caractere
        if (linha[i] >= 32 && linha[i] < 127) {  // Somente caracteres ASCII imprimíveis
            #pragma omp atomic
            freq[linha[i] - 32]++;

            // Depuração: Mostra o caractere, seu código ASCII e a frequência atual
            // printf("letra: '%c', ASCII: %d, index: %d, freq: %d\n", linha[i], linha[i], linha[i] - 32, freq[linha[i] - 32]);
        }
    }
}

void merge(int *freq, int *ascii, int low, int mid, int high) {
    int i = low, j = mid + 1, k = low;
    int temp_freq[high - low + 1];
    int temp_ascii[high - low + 1];

    while (i <= mid && j <= high) {
        if (freq[i] < freq[j] || (freq[i] == freq[j] && ascii[i] < ascii[j])) {
            temp_freq[k - low] = freq[i];
            temp_ascii[k - low] = ascii[i];
            i++;
        } else {
            temp_freq[k - low] = freq[j];
            temp_ascii[k - low] = ascii[j];
            j++;
        }
        k++;
    }

    while (i <= mid) {
        temp_freq[k - low] = freq[i];
        temp_ascii[k - low] = ascii[i];
        i++;
        k++;
    }

    while (j <= high) {
        temp_freq[k - low] = freq[j];
        temp_ascii[k - low] = ascii[j];
        j++;
        k++;
    }

    for (i = low; i <= high; i++) {
        freq[i] = temp_freq[i - low];
        ascii[i] = temp_ascii[i - low];
    }

    for (int i = low; i <= high; i++) {
        printf("%d(%d) ", ascii[i], freq[i]);
    }
    printf("\n");
}

//ordena paralelamente
void merge_sort(int *freq, int *ascii, int low, int high) {
    if (low < high) {
        int mid = (low + high) / 2;
        #pragma omp task
        merge_sort(freq, ascii, low, mid);
        #pragma omp task
        merge_sort(freq, ascii, mid + 1, high);
        #pragma omp taskwait
        merge(freq, ascii, low, mid, high);
    }
}

int main() {    
    // Leitura sequencial de todas as linhas
    while (fgets(linhas[total_linhas], TAMANHO_MAX_LINHA, stdin) != NULL) {
        linhas[total_linhas][strcspn(linhas[total_linhas], "\n")] = '\0';
        total_linhas++;
    }

    // 2. Processamento paralelo
    #pragma omp parallel for
    for (int i = 0; i < total_linhas; i++) {            // a diretiva pragma omp parallel cuida para que não haja condição de corrida na variável i
        int *freq = todas_freq[i];
        int *ascii = todas_ascii[i];
        int flag = 0;

        frequencia(linhas[i], freq);                    // uma tarefa para contar a frequencia de cada letra para cada linha

        for (int j = 0; j < MAXIMO_CARACTERES; j++) {
            if (freq[j] > 0) {
                ascii[flag++] = j + 32;
            }
        }

        ascii_count[i] = flag;

        #pragma omp parallel
        {
            #pragma omp single
            merge_sort(freq, ascii, 0, flag - 1);
        }
    }

    // 3. Impressão sequencial
    for (int i = 0; i < total_linhas; i++) {
        printf("Resultado da linha %d:\n", i + 1);
        for (int j = 0; j < ascii_count[i]; j++) {
            printf("%d %d\n", todas_ascii[i][j], todas_freq[i][todas_ascii[i][j] - 32]);
        }
        printf("\n");
    }

    return 0;
}
