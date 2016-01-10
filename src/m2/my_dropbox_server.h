#define MAX_BUF 1024
#define PORT 6012
#define FILENAME  "./MiNube/"

#define COM_TRF	1
#define COM_DEL	2
#define COM_REN	4

char * comandos[]  = {"CON","TRF", "DEL", "ACK", "REN", "FIN", NULL};

void update(int sock);
int posc( char cad[], char c);
int busca_substring(char *string, char **strings);
char * obtenerTiempo();