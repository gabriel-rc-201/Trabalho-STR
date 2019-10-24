/*	LIVRO FUNDAMENTOS DOS SISTEMAS DE TEMPO REAL
*
*	Implementa uma tarefa periodica usando clock_nanosleep em modo absoluto
*	Compilar com:	gcc -lrt -o tarefaperiodica3 tarefaperiodica3.c
*	ou
*			gcc -o ControleCaldeira ControleCaldeira.c -lrt
*
*	Salva o atraso total ateh o *inicio* da execucao
*	Isto inclui overheads, release jitter e possiveis interferencias
*	Salva tambem o tempo de resposta, entre chegada e conclusao	
*/

#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include "socket.h"

#define	NSEC_PER_SEC    (1000000000) 	// Numero de nanosegundos em um milissegundo
#define	N_AMOSTRAS	100		// Numero de amostras (medicoes) coletadas

void controleTemp(int temperatura_user, int socket_local, struct sockaddr_in endereco_destino){//thread de controle do tempo
	struct timespec t, t_fim;
	long atraso_fim[N_AMOSTRAS];
	int amostra = 0;		// Amostra corrente
	long int periodo = 50000000; 	// 50ms
	
	// Le a hora atual, coloca em t
	clock_gettime(CLOCK_MONOTONIC ,&t);

	// Tarefa periodica iniciará em 1 segundo
	t.tv_sec++;

	while(1) {
		// Espera ateh inicio do proximo periodo
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		// Le a hora atual, coloca em t_inicio

        char* msg_rec = ler("st-0", socket_local, endereco_destino);
        float temperatura_sist = atof(&msg_rec[3]);

	    if (temperatura_user > temperatura_sist){
            altera("ani%lf", 0.0, endereco_destino, socket_local);
            altera("ana%lf", 10.0, endereco_destino, socket_local);            
        }
            
        if(temperatura_user < temperatura_sist){           
            altera("ana%lf", 0.0, endereco_destino, socket_local);
            altera("ani%lf", 10.0, endereco_destino, socket_local);
        }
		
		printf("Passou um periodo de 50ms!\n");	
		// Le a hora atual, coloca em t_fim
		clock_gettime(CLOCK_MONOTONIC ,&t_fim);

		// Calcula o tempo de resposta observado em microsegundos
		atraso_fim[amostra++] = 1000000*(t_fim.tv_sec - t.tv_sec)   +   (t_fim.tv_nsec - t.tv_nsec)/1000;
		// guardar isso direto no buffer usando o bufduplo_insereLeitura( atraso_fim)
		// depois chama a bufduplo_esperaBufferCheio(void) para gravar logo o arquivo


		// Calcula inicio do proximo periodo
		t.tv_nsec += periodo;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}
	}
	printf("Terminou ControleTemperatura");
}

void controleAltura(float max_h, int socket_local, struct sockaddr_in endereco_destino){// thread de controle da altura
	struct timespec t;
	long int periodo = 70000000; 	// 70ms
	
	// Le a hora atual, coloca em t
	clock_gettime(CLOCK_MONOTONIC ,&t);

	// Tarefa periodica iniciará em 1 segundo
	t.tv_sec++;

	while(1) {
		// Espera ateh inicio do proximo periodo
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

        char* msg_rec = ler("sh-0", socket_local, endereco_destino);
        float haltura = atof(&msg_rec[3]);
        
	    if(haltura >= max_h){
            altera("anf%lf", 100.0, endereco_destino, socket_local);
            altera("ani%lf", 0.0, endereco_destino, socket_local);
            altera("ana%lf", 0.0, endereco_destino, socket_local);    
        }
                
        if(haltura <= 1){
            altera("anf%lf", 0, endereco_destino, socket_local);
        }
		printf("Passou um periodo de 70ms!\n");	

		// Calcula inicio do proximo periodo
		t.tv_nsec += periodo;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}
	}
	printf("Terminou ControleAltura");
}
 void printaTela(){
	 /*
	 	macho faz aki o ponto 4 q eu vou aproveitar q tu ta chamando pra fazer a armazenagem no buffer logo
	 */
 }
int main(int argc, char* argv[]){

	struct timespec t, t_inicio, t_fim;
     if (argc < 3) { 
		fprintf(stderr,"Uso: controlemanual <endereco> <porta>\n");
		fprintf(stderr,"<endereco> eh o endereco IP da caldeira\n");
		fprintf(stderr,"<porta> eh o numero da porta UDP da caldeira\n");
		fprintf(stderr,"Exemplo de uso:\n");
		fprintf(stderr,"   controlemanual localhost 12345\n");
		exit(1);
	}

    int porta_destino = atoi(argv[2]);
    
    int socket_local = cria_socket_local();

	struct sockaddr_in endereco_destino = cria_endereco_destino(argv[1], porta_destino);

	controleTemp(25, socket_local, endereco_destino);
	controleAltura(2.5, socket_local, endereco_destino);

}

