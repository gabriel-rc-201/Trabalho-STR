
int cria_socket_local(void);

struct sockaddr_in cria_endereco_destino(char *destino, int porta_destino);

void envia_mensagem(int socket_local, struct sockaddr_in endereco_destino, char *mensagem);

int recebe_mensagem(int socket_local, char *buffer, int TAM_BUFFER);

char* ler(char* consulta, int socket_local, struct sockaddr_in endereco_destino);//ler as variaveis do simulador;

void altera(char* consulta, float valor, struct sockaddr_in endereco_destino, int socket_local);//altera os valores das variaveis no simulador
