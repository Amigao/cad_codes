// para compilar: gcc token_multiple_ring.c -o token_multiple_ring -pthread
// para executar: token_multiple_ring

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define T 9
#define ROUNDS 5
int vlr = 0;

int dir = 1;

// sem_wait
// sem_post

sem_t token[4];

void *increment(void *arg) {
    int id = *((int *)arg);
    free(arg); // Liberar memória alocada

    for (int i = 0; i < ROUNDS; i++){
        sem_wait(&token[id]);
        vlr++;
        printf("[THREAD %d] VALOR ATUALIZADO: %d\n\n", id, vlr);

        if ((id == T - 1 && dir == 1) || (id == 1 && dir == -1))
            sem_post(&token[0]);
        else
            sem_post(&token[id + dir]);
        
    }

	pthread_exit(0);
}


int main(void) {
	pthread_t threads[T];

    for (int i = 0; i < T; i++) {
        sem_init(&token[i], 0, 0);
    }	
    
    for (int i = 1; i < T; i++){
        int *arg = malloc(sizeof(int));
        *arg = i;
        if (pthread_create(&threads[i], NULL, increment, arg) != 0) {
            printf("[ERRO] Falha ao criar a thread produtora!\n");
            exit(1);
        }
    }
	
    printf("[PAI] Aguardando threads finalizarem...\n\n");
    sem_post(&token[1]);

    for (int i = 0; i < ROUNDS; i++){
        sem_wait(&token[0]);
        if (dir == 1) sem_post(&token[T - 1]);
        else if (dir == -1) sem_post(&token[1]);
        dir *= -1;
        vlr++;
        printf("[THREAD 0] VALOR ATUALIZADO: %d\n\n", vlr);
    }

    // Aguarda as threads terminarem
    for (int i = 1; i < T; i++) {
        pthread_join(threads[i], NULL);
    }	

    printf("[PAI] Execução concluída. Encerrando programa. Valor final: %d\n", vlr);
	exit(0);	
} // main()

// ia generativa
// sugestoes do celular
// videogame
// anuncios
// email