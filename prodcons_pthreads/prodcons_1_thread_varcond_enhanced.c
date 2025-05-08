// Compilação: gcc prodcons_1_thread_varcond_enhanced.c -o prodcons_1_thread_varcond_enhanced -lpthread
// Execução: ./prodcons_1_thread_varcond_enhanced
#include <stdlib.h>  // Para rand() e srand()
#include <unistd.h>  // Para usleep()
#include <time.h>    // Para seed aleatória (srand)

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MAX_PRODUCED 25 // Número máximo de itens a serem produzidos
#define MAX_QUEUE 7     // Tamanho máximo da fila


// Variáveis de sincronização
pthread_cond_t cond_queue_empty, cond_queue_full;
pthread_mutex_t item_queue_cond_lock;

// Fila compartilhada e contadores
int queue[MAX_QUEUE], item_available = 0, produced = 0, consumed = 0;
int producers = 15, consumers = 6;

// Função para criar um item aleatório
int create_item(void) {
    return rand() % 1000; // Retorna um número aleatório entre 0 e 999
}

// Função para inserir um item na fila
void insert_into_queue(int item, int id) {
    queue[item_available++] = item; // Insere o item e incrementa a posição disponível
    produced++;
    printf("[PRODUTOR %d] Item %d gerado (Valor: %d). Itens na fila: %d\n", id, produced, item, item_available);
}

// Função para remover um item da fila
int extract_from_queue(int id) {
    consumed++;
    item_available--;
    int item = queue[item_available]; // Remove o último item da fila
    printf("[CONSUMIDOR %d] Item %d consumido (Valor: %d). Itens restantes na fila: %d\n", id, consumed, item, item_available);
    return item;
}

// Função para processar um item consumido
void process_item(int my_item) {
    static int processed = 0;
    // printf("[PROCESSAMENTO] Item %d processado (Valor: %d). Itens na fila: %d\n", ++processed, my_item, item_available);
}

// Função do produtor
void *producer(void *arg) {
    int id = *((int *)arg);
    free(arg); // Liberar memória alocada
    int item;
    while (1) {
        item = create_item();
        pthread_mutex_lock(&item_queue_cond_lock);
        // Aguarda se a fila estiver cheia
        while (item_available == MAX_QUEUE && produced < MAX_PRODUCED)
            pthread_cond_wait(&cond_queue_empty, &item_queue_cond_lock);
        
            if (produced >= MAX_PRODUCED){
                pthread_mutex_unlock(&item_queue_cond_lock);
                pthread_cond_signal(&cond_queue_full); // Sinaliza que há itens disponíveis
                break;
            }
            insert_into_queue(item, id);
        
        pthread_cond_signal(&cond_queue_full); // Sinaliza que há itens disponíveis
        pthread_mutex_unlock(&item_queue_cond_lock);
        usleep(1000 + rand() % 1000);  // Pausa entre 1ms e 2ms
    }
    printf("[PRODUTOR %d] Produção finalizada.\n", id);
    pthread_exit(0);
}

// Função do consumidor
void *consumer(void *arg) {
    int id = *((int *)arg);
    free(arg); // Liberar memória alocada
    int my_item;
    while (1) {
        pthread_mutex_lock(&item_queue_cond_lock);
        // Aguarda se a fila estiver vazia
        while (item_available == 0 && produced < MAX_PRODUCED)
            pthread_cond_wait(&cond_queue_full, &item_queue_cond_lock);
        
        if (consumed >= MAX_PRODUCED){
        	pthread_mutex_unlock(&item_queue_cond_lock);
            pthread_cond_signal(&cond_queue_empty); // Sinaliza que há espaço na fila
			break;
		}

        my_item = extract_from_queue(id);
        
        pthread_cond_signal(&cond_queue_empty); // Sinaliza que há espaço na fila
        pthread_mutex_unlock(&item_queue_cond_lock);
        
        process_item(my_item);
        usleep(1000 + rand() % 1000);  // Pausa entre 1ms e 2ms
    }
    printf("[CONSUMIDOR %d] Consumo finalizado.\n", id);
    pthread_exit(0);
}

int main(void) {
    srand(time(NULL));
    
    pthread_t prod_handle[producers], cons_handle[consumers];

    // Inicialização das variáveis de sincronização
    pthread_cond_init(&cond_queue_empty, NULL);
    pthread_cond_init(&cond_queue_full, NULL);
    pthread_mutex_init(&item_queue_cond_lock, NULL);

    // Criação das threads do produtor e do consumidor
    for (int i = 0; i < producers; i++){
		int *arg = malloc(sizeof(int));
        *arg = i;
		if (pthread_create(&prod_handle[i], NULL, producer, arg) != 0) {
			printf("[ERRO] Falha ao criar a thread do produtor. Encerrando!\n");
			exit(1);
		}
	}
    for (int i = 0; i < consumers; i++){
		int *arg = malloc(sizeof(int));
        *arg = i;
		if (pthread_create(&cons_handle[i], NULL, consumer, arg) != 0) {
			printf("[ERRO] Falha ao criar a thread do consumidor. Encerrando!\n");
			exit(1);
		}
	}

    // Aguarda as threads terminarem
    for (int i = 0; i < consumers; i++) {
        pthread_join(cons_handle[i], NULL);
    }

    for (int i = 0; i < producers; i++) {
        pthread_join(prod_handle[i], NULL);
    }

    printf("[FINALIZADO] Produção e consumo concluídos.\n");
    return 0;
}