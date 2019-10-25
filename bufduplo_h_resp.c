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
FILE* dados_arq;

static void insere_dado_arq(int dado){ 
    dados_arq = fopen("dados_nivel.txt", "a");
    if(dados_arq == NULL){
        printf("Erro, nao foi possivel abrir o arquivo\n");
        exit(1);    
    }	
    fprintf(dados_arq, "%d\n",dado);
    fclose(dados_arq); 
       
}
void bufduplo_insereLeitura_h( double leitura) {
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

void bufduplo_esperaBufferCheio_h(void) {
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
	//alterar para ao inves de retornar no buffer ja gravar no 
	
	for( int i=0; i<TAMBUF; ++i){
        insere_dado_arq(buffer[i]);
    }
	
	pthread_t buffer_h;
	pthread_create(&buffer_h, NULL, (void *) bufduplo_esperaBufferCheio_h, NULL);
	pthread_join( buffer_h, NULL);

	

}

