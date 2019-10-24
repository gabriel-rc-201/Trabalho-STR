/* Monitor buffer duplo, no arquivo bufduplo.c */
#include	<pthread.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#define TAMBUF 100 
static double buffer_0[TAMBUF]; 
static double buffer_1[TAMBUF];

static int emuso = 0; 
static int prox_insercao = 0; 
static int gravar = -1;

static pthread_mutex_t exclusao_mutua = PTHREAD_MUTEX_INITIALIZER; 
static pthread_cond_t buffer_cheio = PTHREAD_COND_INITIALIZER;

void bufduplo_insereLeitura( double leitura) {
	pthread_mutex_lock( &exclusao_mutua); 
	if( emuso == 0) 
		buffer_0[prox_insercao] = leitura; 
	else 
		buffer_1[prox_insercao] = leitura; 
	++prox_insercao; 
	if( prox_insercao == TAMBUF){ 
		gravar = emuso; 
		emuso = (emuso + 1)% 2; 
		prox_insercao = 0; 
		pthread_cond_signal( &buffer_cheio); 
	} 
	pthread_mutex_unlock( &exclusao_mutua);
}

double *bufduplo_esperaBufferCheio(void) {
	double *buffer; 
	pthread_mutex_lock( &exclusao_mutua); 
	while( gravar == -1) 
		pthread_cond_wait( &buffer_cheio, &exclusao_mutua);
	if(gravar==0) 
		buffer = buffer_0; 
	else 
		buffer = buffer_1; 
	gravar = -1; 
	pthread_mutex_unlock( &exclusao_mutua); 
	//alterar para ao inves de retornar no buffer ja gravar no arquivos
	// como começar sempre do fim do arquivo?
	return buffer; 
}

