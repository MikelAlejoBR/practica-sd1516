#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

#include "protocolo.h"
struct stat fileinfo
int sendack(int sock) {
	int bsent;
	bsent = write(sock, "ACK", sizeof("ACK"));
	if(bsent < sizeof("ACK"))
		return(-1);
	return 0;
}

int senderr(int sock, int ecode) {
	char errbuff[7];
	snprintf(errbuff, sizeof(errbuff), "ERR %d", ecode);
	
	int bsent;
	bsent = write(sock, errbuff, sizeof(errbuff));
	
	if(bsent < sizeof(errbuff))
		return(-1);
	return 0;
}

int sendFile(int sock, char * file){
    int bsent;
    if(stat(param, &fileinfo)<0)
        fprintf(stderr,"Fichero no encontrado\n");
    else
    {
        bsent=write(sock, "TRF;" , sizeof("TRF;"));
        if(bsent < sizeof(errbuff))
            return(-1);
        bsent=write(sock, file, sizeof(file));
        if(bsent < sizeof(errbuff))
            return(-1);
        bsent=write(sock, ";" , sizeof(";"));
        if(bsent < sizeof(errbuff))
            return(-1);
        int tam= fileinfo.st_size;
        char str[20];
        sprintf(str,tam);
        bsent=write(sock, str , sizeof(str));
        if(bsent < sizeof(errbuff))
            return(-1);
        bsent=write(sock, ";" , sizeof(";"));
        if(bsent < sizeof(errbuff))
            return(-1);
        /*enviar fichero a trozos*/
        
        
    
}

int sendfin(int sock) {
	int bsent;
	bsent = write(sock, "FIN", sizeof("FIN"));
	if(bsent < sizeof("FIN"))
		return(-1);
	return 0;
}

int getsockfd(){
	int sock;
	struct sockaddr_in serv_info;
	
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error al crear el socket");
		return(-1);
	}

	memset(&serv_info, 0, sizeof(serv_info));
	serv_info.sin_family = AF_INET;
	serv_info.sin_port = htons(PORT);
	
	if(inet_aton("127.0.0.1", &serv_info.sin_addr) <= 0) {
		fprintf(stderr, "Error en la direccion IP: 127.0.0.1\n");
		return(-1);
	}
	
	if(connect(sock, (struct sockaddr *) &serv_info, sizeof(serv_info)) < 0) {
		perror("Error al intentar conectar con el servidor");
		return(-1);
	}

	return sock;
}

int releasesockfd(int sock) {
	if(close(sock) < 0) {
		fprintf(stderr, "Error al cerrar el socket");
		return(-1);
	}
	return 0;
}