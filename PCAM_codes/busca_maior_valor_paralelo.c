// to compile: gcc 07-05-encontra_max_omp_barrier_solicitado_alunos.c -o 07-05-encontra_max_omp_barrier_solicitado_alunos -fopenmp
// to execute: 07-05-encontra_max_omp_barrier_solicitado_alunos <num_elements>

#include <stdlib.h>
#include <stdio.h>
#include <omp.h>



int main(int argc,char **argv)
{

// maior = -1 tem que estar definido antes da regiao paralela. 
// Forma uma condicao de disputa na RC, se etiver sem protecao do lock.
int i, *vetor, maior=-1, tam;
double wtime;

// a definicao dos locks tem que ser antes da regiao paralela
// eles estavam dentro da regiao paralela (2021/1)
omp_lock_t mylock;
omp_init_lock(&mylock);

if ( argc  != 2)
{
	printf("Wrong arguments. Please use main <amount_of_elements>\n");
	exit(0);
}

tam = atoi(argv[1]);

printf("Amount of vetor=%d\n", tam);
fflush(0);

int num_threads = omp_get_num_procs();
omp_set_num_threads(num_threads);

vetor=(int*)malloc(tam*sizeof(int)); //Aloca o vetor da dimensão lida

// getting start time after malloc.
wtime = omp_get_wtime();


    #pragma omp parallel shared (maior)
    {
		int i, my_id, my_range, my_first_i, my_last_i;
		int localmaior;

		// determina o nr da thread
		my_id = omp_get_thread_num();

		my_range = tam/num_threads;
		
		my_first_i = my_range * my_id;
		
		if (my_id < num_threads-1)
		{
			my_last_i = my_first_i + my_range; // na verdade eh o primeiro do proximo range
		}  // fim do then
		else
		{
			my_last_i = tam; // a ultima thread fica com o resto, caso divisao N/num_threads não for exata
		} // fim do else

		// iniciando vetor e fixando o maior valor para validacao
		#pragma omp simd
		for(i = my_first_i; i < my_last_i; i++)
		{
			vetor[i] = 1;
		} // fim do for

		// barreira tem que estar antes do single, para impedir que a escrita de tam
		// na posicao tam/2 do vetor seja sobrescrita por outra thread
		// sync point
		#pragma omp barrier

		#pragma omp single
		{
			vetor[tam/2] = tam;
		}
		localmaior = -1;
		
		for(i = my_first_i; i < my_last_i; i++)
		{
			if(vetor[i] > localmaior)
			{
				localmaior=vetor[i];
			} 
		} 
	
		omp_set_lock(&mylock);
		if (localmaior > maior)
		{
			maior = localmaior;
		}
		omp_unset_lock(&mylock);

    }  // end parallel region

    wtime = omp_get_wtime() - wtime;

/*  // imprimindo o vetor para conferencia
    for(i = 0; i < tam; i++)
    {
	printf("vetor[%d]=%d\n",i,vetor[i]);
    }
    printf("\n");
*/
	
    printf("OMP Barrier: Tam=%d, Num_Threads=%d, maior=%d, Elapsed wall clock time = %f  \n", tam, num_threads, maior, wtime);
    free(vetor); //Desaloca o vetor
	
    return 0;
} // fim da main
