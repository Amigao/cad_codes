/*
Trabalho 1: Frequência de Caracteres (SSC0903-CAD - 2025/1)
Integrantes:
- Beatriz Cardoso de Oliveira - NUSP 12566400
- Daniel Dias Silva Filho     - NUSP 13677114
- Pedro Arthur Francoso       - NUSP 12547301
- João Victor de Almeida      - NUSP 13695424
*/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXIMO_CARACTERES 96   // total ascii fornecido
#define TAMANHO_MAX_LINHA 1000 // tamanho maximo de cada linha
#define QTD_TOTAL_LINHAS 1    // quantidade total de linhas

int todas_freq[QTD_TOTAL_LINHAS][MAXIMO_CARACTERES];  // Frequências por linha
int todas_ascii[QTD_TOTAL_LINHAS][MAXIMO_CARACTERES]; // ASCII por linha
char linhas[QTD_TOTAL_LINHAS][TAMANHO_MAX_LINHA];     // Linhas
int ascii_count[QTD_TOTAL_LINHAS]; // Quantidade de letras distintas por linha
int total_linhas = 0;

void frequencia(char *linha, int *freq) {
  size_t tamanho_string = strlen(linha);
  #pragma omp parallel for
  for (int i = 0; i < tamanho_string; i++) { // Uma tarefa pra cada caractere
    if (linha[i] >= 32 &&
        linha[i] < 127) { // Somente caracteres ASCII imprimiveis
      #pragma omp atomic
      freq[linha[i] - 32]++; // Incrementa a frequencia do caractere
    }
  }
}

void merge(int *freq, int *ascii, int low, int mid, int high) {
  int i = low, j = mid + 1, k = low;
  int temp_freq[high - low + 1];
  int temp_ascii[high - low + 1];

  // Enquanto houver elementos em ambos os subarrays
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

  // Enquanto houver elementos no subarray da primeira metade do array
  while (i <= mid) {
    temp_freq[k - low] = freq[i];
    temp_ascii[k - low] = ascii[i];
    i++;
    k++;
  }

  // Enquanto houver elementos no subarray da segunda metade do array
  while (j <= high) {
    temp_freq[k - low] = freq[j];
    temp_ascii[k - low] = ascii[j];
    j++;
    k++;
  }

  // Atualiza as frequências e os valores ASCII no array original
  for (i = low; i <= high; i++) {
    freq[i] = temp_freq[i - low];
    ascii[i] = temp_ascii[i - low];
  }
}

// Ordena paralelamente o array de frequências e os valores ASCII
// correspondentes
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
  for (int i = 0; i < total_linhas;
       i++) { // a diretiva pragma omp parallel cuida para que não haja condição
              // de corrida na variável i
    int *freq = todas_freq[i];
    int *ascii = todas_ascii[i];
    int flag = 0;

    frequencia(linhas[i], freq); // uma tarefa para contar a frequencia de cada
                                 // letra para cada linha

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
      printf("%c %d\n", todas_ascii[i][j],
             todas_freq[i][todas_ascii[i][j] - 32]);
    }
    printf("\n");
  }

  return 0;
}
