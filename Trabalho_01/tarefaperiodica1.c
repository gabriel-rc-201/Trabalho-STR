/*	
*           Dupla:
*                   Gabriel de Souza 398847
*                   Gabriel Ribeiro 401091
*			gcc -o tarefaperiodica1 tarefaperiodica1.c -lrt
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


#define NSEC_PER_SEC    (1000000000) 	// Numero de nanosegundos em um segundo

#define FALHA 1


char teclado[1000];
double valor;
char msg_enviada[1000], msg_enviada_clone[1000];  
char msg_recebida[1000];
int nrec;

int cria_socket_local(void)
{
	int socket_local;		/* Socket usado na comunicacao */

	socket_local = socket( PF_INET, SOCK_DGRAM, 0);
	if (socket_local < 0) {
		perror("socket");
		return -1;
	}
	return socket_local;
}


struct sockaddr_in cria_endereco_destino(char *destino, int porta_destino)
{
	struct sockaddr_in servidor;	/* Endereco do servidor incluindo ip e porta */
	struct hostent *dest_internet;	/* Endereco destino em formato proprio */
	struct in_addr dest_ip;			/* Endereco destino em formato ip numerico */

	if (inet_aton ( destino, &dest_ip ))
		dest_internet = gethostbyaddr((char *)&dest_ip, sizeof(dest_ip), AF_INET);
	else
		dest_internet = gethostbyname(destino);

	if (dest_internet == NULL) {
		fprintf(stderr,"Endereco de rede invalido\n");
		exit(FALHA);
	}

	memset((char *) &servidor, 0, sizeof(servidor));
	memcpy(&servidor.sin_addr, dest_internet->h_addr_list[0], sizeof(servidor.sin_addr));
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(porta_destino);

	return servidor;
}


void envia_mensagem(int socket_local, struct sockaddr_in endereco_destino, char *mensagem)
{
	/* Envia msg ao servidor */

	if (sendto(socket_local, mensagem, strlen(mensagem)+1, 0, (struct sockaddr *) &endereco_destino, sizeof(endereco_destino)) < 0 )
	{ 
		perror("sendto");
		return;
	}
}


int recebe_mensagem(int socket_local, char *buffer, int TAM_BUFFER)
{
	int bytes_recebidos;		/* Numero de bytes recebidos */

	/* Espera pela msg de resposta do servidor */
	bytes_recebidos = recvfrom(socket_local, buffer, TAM_BUFFER, 0, NULL, 0);
	if (bytes_recebidos < 0)
	{
		perror("recvfrom");
	}

	return bytes_recebidos;
}


int main(int argc, char* argv[])
{

    if (argc < 3) { 
		fprintf(stderr,"Uso: controlemanual <endereco> <porta>\n");
		fprintf(stderr,"<endereco> eh o endereco IP da caldeira\n");
		fprintf(stderr,"<porta> eh o numero da porta UDP da caldeira\n");
		fprintf(stderr,"Exemplo de uso:\n");
		fprintf(stderr,"   controlemanual localhost 12345\n");
		exit(FALHA);
	}

    int porta_destino = atoi( argv[2]);
    
    int socket_local = cria_socket_local();

	struct sockaddr_in endereco_destino = cria_endereco_destino(argv[1], porta_destino);
    

	struct timespec t;
	long int periodo = 30000000; 	// 30ms

	// Le a hora atual, coloca em t
	clock_gettime(CLOCK_MONOTONIC ,&t);

	// Tarefa periodica iniciará em 1 segundo
	t.tv_sec++;
    
    int temperatura_user;
    float haltura, temperatura_sist;

    printf("digite o valor da temperatura desejada\n");
    scanf("%d", &temperatura_user);
    
    char* ler(char* consulta){//ler as variaveis do simulador
        strcpy( msg_enviada, consulta);        
		
        envia_mensagem(socket_local, endereco_destino, msg_enviada);

		nrec = recebe_mensagem(socket_local, msg_recebida, 1000);
		msg_recebida[nrec] = '\0';

        return msg_recebida;
    }
    

    void altera(char* consulta, float valor){//altera os valores das variaveis no simulador
        sprintf( msg_enviada, consulta, valor);        
		
        envia_mensagem(socket_local, endereco_destino, msg_enviada);

		nrec = recebe_mensagem(socket_local, msg_recebida, 1000);
		msg_recebida[nrec] = '\0';
    }
    
    void altera_altura(){//equilibra a altura entre 2.6 e 2 m
         if(haltura >= 2.6){
                altera("anf%lf", 100.0);
                altera("ani%lf", 0.0);
                altera("ana%lf", 0.0);    
            }
            
            if(haltura <= 2){
                altera("anf%lf", 0);            
            }
    }
	while(1) {
		// Espera ateh inicio do proximo periodo
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

        // Realiza seu trabalho
        
        char* msg_rec = ler("st-0");

        temperatura_sist = atof(&msg_rec[3]);
        printf(">>>%f<<<\n", temperatura_sist);
		
        if (temperatura_user > temperatura_sist){
            msg_rec = ler("sh-0");
            haltura = atof(&msg_rec[3]);
            
            altera_altura();

            altera("ani%lf", 0.0);
            altera("ana%lf", 10.0);            
        }
        
        if(temperatura_user < temperatura_sist){
            msg_rec = ler("sh-0");
            haltura = atof(&msg_rec[3]);

            altera_altura();
            
            altera("ana%lf", 0.0);
            altera("ani%lf", 10.0);
        }

		// Calcula inicio do proximo periodo
		t.tv_nsec += periodo;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}
	}
}

