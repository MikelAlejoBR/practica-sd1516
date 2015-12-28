#define MAX_BUF 1024
#define PORT 6012

#define ST_INIT	0

#define COM_TRF	1
#define COM_DEL	2
#define COM_FIN	5

char * comandos[]  = {"CON","TRF", "DEL", "ACK", "ERR", "FIN", NULL};

int estado;

void sesion(int sock);
int busca_string(char *string, char **strings);
int busca_substring(char *string, char **strings);