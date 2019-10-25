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
#include <pthread.h>
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
#include "bufduplo_t_resp.h"
#include "bufduplo_h_resp.h"
#include "bufduplo_sensores.h"

#define	NSEC_PER_SEC    (1000000000) 	// Numero de nanosegundos em um milissegundo
#define	N_AMOSTRAS	100		// Numero de amostras (medicoes) coletadas

int porta_destino;
    
int socket_local;

struct sockaddr_in endereco_destino;

float max_h;
int temperatura_user;

void controleTemp(){//thread de controle do tempo
	struct timespec t, t_fim;
	long int periodo = 50000000; 	// 50ms
	
	// Le a hora atual, coloca em t
	clock_gettime(CLOCK_MONOTONIC ,&t);

	// Tarefa periodica iniciará em 1 segundo
	t.tv_sec++;

	while(1) {
		// Espera ateh inicio do proximo periodo
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

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
		
		//printf("Passou um periodo de 50ms!\n");	
		// Le a hora atual, coloca em t_fim
		clock_gettime(CLOCK_MONOTONIC ,&t_fim);

		// Calcula o tempo de resposta observado em microsegundos
		bufduplo_insereLeitura_t(1000000*(t_fim.tv_sec - t.tv_sec)   +   (t_fim.tv_nsec - t.tv_nsec)/1000);
		// Calcula inicio do proximo periodo
		t.tv_nsec += periodo;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}
	}
	printf("Terminou ControleTemperatura");
}

void controleAltura(){// thread de controle da altura
	struct timespec t, t_fim;
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
                
        if(haltura <= 1.8){
            altera("anf%lf", 0, endereco_destino, socket_local);
        }
		//printf("Passou um periodo de 70ms!\n");	
		
		// Le a hora atual, coloca em t_fim
		clock_gettime(CLOCK_MONOTONIC ,&t_fim);

		bufduplo_insereLeitura_h(1000000*(t_fim.tv_sec - t.tv_sec)   +   (t_fim.tv_nsec - t.tv_nsec)/1000);
		
		// Calcula inicio do proximo periodo
		t.tv_nsec += periodo;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}
	}
	printf("Terminou ControleAltura");
}
  void printaTela(){//ponto 4
	char *Ta, *T, *Ti, *No, *H;
	float ta, t, ti, no, h;
	
	while (1){
		Ta = ler("sta0",socket_local, endereco_destino);// Temperatura Ambiente
		ta = atof(&Ta[3]);
		printf("[Ta] Temperatura do ar  ambiente em volta do recipiente [Grau Celsius]: %f\n",ta );
		bufduplo_insereLeitura_s(ta);
		T = ler("st-0", socket_local, endereco_destino);// Temperatura da agua dentro do recipiente
		t = atof(&T[3]);
		printf("[T]  Temperatura da água no interior do recipiente [Grau Celsius]: %f\n", t);
		bufduplo_insereLeitura_s(t);
		Ti = ler("sti0", socket_local, endereco_destino);// Temperatura da agua que entra no
		ti = atof(&Ti[3]);
		printf("[Ti] Temperatura da água que entra no recipiente [Grau Celsius]: %f\n", ti); 
		bufduplo_insereLeitura_s(ti);
		No = ler("sno0", socket_local, endereco_destino);// Fluxo de saida de agua
		no = atof(&No[3]);
		printf("[No] Fluxo de água de saída do recipiente [Kg/segundo]: %f\n", no );
		bufduplo_insereLeitura_s(no);
		H = ler("sh-0", socket_local, endereco_destino);// altura da coluna de agua dentro do recipiente
		h =  atof(&H[3]);
		printf("[H]  Altura da coluna de água dentro do recipiente [m]: %f\n",h);
		bufduplo_insereLeitura_s(h);
		
		// ele printa a partir do 3 para pular o "sh-" 
		//printando somente os valores 
		sleep(1);
		system("clear");
	}
	
 }

int main(int argc, char* argv[]){

	struct timespec t, t_inicio, t_fim;
     if (argc < 3) { 
		fprintf(stderr,"Uso: ControleCaldeira <endereco> <porta>\n");
		fprintf(stderr,"<endereco> eh o endereco IP da caldeira\n");
		fprintf(stderr,"<porta> eh o numero da porta UDP da caldeira\n");
		fprintf(stderr,"Exemplo de uso:\n");
		fprintf(stderr,"   ControleCaldeira localhost 12345\n");
		exit(1);
	}

    porta_destino = atoi(argv[2]);
    
    socket_local = cria_socket_local();

	endereco_destino = cria_endereco_destino(argv[1], porta_destino);

	pthread_t temp, nivel,tela, bufferT, buffer_h, buffer_s;
	
	printf("Digite o valor desejado da temperatura:\n");
	scanf("%d", &temperatura_user);
	printf("Digite o valor desejado do nivel maximo de agua:\n");
	scanf("%f", &max_h);

    pthread_create(&temp, NULL, (void *) controleTemp, NULL);
    pthread_create(&nivel, NULL, (void *) controleAltura, NULL);
	pthread_create(&tela, NULL, (void *) printaTela, NULL);
	pthread_create(&bufferT, NULL, (void *) bufduplo_esperaBufferCheio_t, NULL);
	pthread_create(&buffer_h, NULL, (void *) bufduplo_esperaBufferCheio_h, NULL);
	pthread_create(&buffer_s, NULL, (void *) bufduplo_esperaBufferCheio_s, NULL);

	pthread_join( temp, NULL);
	pthread_join( nivel, NULL);
	pthread_join( tela, NULL);
	pthread_join( bufferT, NULL);
	pthread_join( buffer_h, NULL);
	pthread_join( buffer_s, NULL);
}

