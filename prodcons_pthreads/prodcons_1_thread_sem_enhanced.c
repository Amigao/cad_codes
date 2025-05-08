#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> // Para sleep()

#define MAX_PRODUCED 27 // Número máximo de itens a serem produzidos
#define MAX_QUEUE 7     // Capacidade máxima da fila

sem_t mutex, empty, full; // Semáforos para controle de acesso
sem_t producers_sem, consumers_sem; // Semáforos para controle dos produtores e consumidores

int queue[MAX_QUEUE], item_available = 0, produced = 0, consumed = 0;

int producers = 10, consumers = 15;

// Função que gera um item aleatório
int create_item(void) {
    return rand() % 1000;
}

// Insere um item na fila
void insert_into_queue(int item, int producer) {
    queue[item_available++] = item;
    produced++;
    printf("[PRODUTOR %d] Produziu item %d (Valor: %d). Itens na fila: %d\n", producer, produced, item, item_available);
}

// Retira um item da fila
int extract_from_queue(int consumer) {
    consumed++;
    int item = queue[--item_available];
    printf("[CONSUMIDOR %d] Consumiu item %d (Valor: %d). Itens restantes na fila: %d\n", consumer, consumed, item, item_available);
    return item;
}

// Processa um item consumido
void process_item(int my_item) {
    static int processed = 0;
    printf("[PROCESSO] Item %d processado (Valor: %d). Itens na fila: %d\n", processed++, my_item, item_available);
}

// Função da thread produtora
void *producer(void *arg) {
    int id = *((int *)arg);
    free(arg); // Liberar memória alocada
    while (1) {
        sem_wait(&producers_sem);
        if (produced >= MAX_PRODUCED) {
            sem_post(&producers_sem);
            break;
        }
        int item = create_item();
        sem_wait(&empty);  // Aguarda espaço na fila
        sem_wait(&mutex);  // Garante acesso exclusivo
        insert_into_queue(item, id);
        sem_post(&mutex);  // Libera acesso
        sem_post(&full);   // Indica que há um novo item disponível
        sem_post(&producers_sem);
        usleep(50000); // Pequeno atraso para facilitar visualização
    }
    printf("[PRODUTOR %d] Finalizando produção.\n\n", id);
    pthread_exit(0);
}

// Função da thread consumidora
void *consumer(void *arg) {
    int id = *((int *)arg);
    free(arg); // Liberar memória alocada
    while (1) {
        sem_wait(&consumers_sem);
        if (consumed >= MAX_PRODUCED){
            sem_post(&consumers_sem);
            break;
        } 
        sem_wait(&full);   // Aguarda um item na fila
        sem_wait(&mutex);  // Garante acesso exclusivo
        int my_item = extract_from_queue(id);
        sem_post(&mutex);  // Libera acesso
        sem_post(&empty);  // Indica que há espaço disponível
        sem_post(&consumers_sem);
        // process_item(my_item);
        usleep(70000); // Pequeno atraso para facilitar visualização
    }
    printf("[CONSUMIDOR %d] Finalizando consumo.\n\n", id);
    pthread_exit(0);
}

int main(void) {
    pthread_t prod_handles[producers], cons_handles[consumers];

    // Inicializa os semáforos
    sem_init(&mutex, 0, 1);    // Controle de acesso à fila
    sem_init(&empty, 0, MAX_QUEUE); // Indica espaços vazios na fila
    sem_init(&full, 0, 0);     // Indica itens disponíveis na fila
    
    sem_init(&consumers_sem, 0, 1);    // Controle de acesso à fila
    sem_init(&producers_sem, 0, 1);    // Controle de acesso à fila

    printf("[PAI] Criando threads produtor e consumidor...\n\n");

    for (int i = 0; i < producers; i++){
        int *arg = malloc(sizeof(int));
        *arg = i;
        if (pthread_create(&prod_handles[i], NULL, producer, arg) != 0) {
            printf("[ERRO] Falha ao criar a thread produtora!\n");
            exit(1);
        }
    }

    for (int i = 0; i < consumers; i++){
        int *arg = malloc(sizeof(int));
        *arg = i;
        if (pthread_create(&cons_handles[i], NULL, consumer, arg) != 0) {
            printf("[ERRO] Falha ao criar a thread consumidora!\n");
            exit(1);
        }
    }

    printf("[PAI] Aguardando threads finalizarem...\n\n");

    // Aguarda as threads terminarem
    for (int i = 0; i < producers; i++) {
        pthread_join(prod_handles[i], NULL);
    }

    for (int i = 0; i < consumers; i++) {
        pthread_join(cons_handles[i], NULL);
    }

    printf("[PAI] Execução concluída. Encerrando programa.\n");
    return 0;
}
