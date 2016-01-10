#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

#include <sys/ioctl.h>

#include "protocolo.h"



struct stat fileinfo;
#define MAX_BUF 1024
char buf[MAX_BUF];

/*
 * waitforack
 * @param int sock
 * 
 * Waits for an ACK message of the server
 * 
 * @return -1 if something different from ACK was received
 * @return 0 if TIMEOUT was reached
 * @return 1 if ACK was received
 */
int waitforack(int sock) {
		int returnvalue, acklength;
		acklength = strlen("ACK");
		char buffer[acklength];
	
		fd_set set;
		struct timeval timeout;
		
		FD_ZERO(&set);
		FD_SET(sock, &set);
		
		timeout.tv_sec = TIMEOUT;
		timeout.tv_usec = 0;
		
		returnvalue = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		
		if (!returnvalue)
			return(0);
		
		int length = 0;
		ioctl(sock, FIONREAD, &length);
		
		if(length == acklength)
			length = read(sock, buffer, length);
		else
			return(-1);
		
		if (strcmp("ACK", buffer))
			return(-1);
		else
			return(1);
}

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

/*int sendFile(int sock, char * file){
    int bsent;
    if(stat(param, &fileinfo)<0)
        fprintf(stderr,"Fichero no encontrado\n");
    else
    {
        bsent=write(sock, "TRF;" , sizeof("TRF;"));
        if(bsent < sizeof("TRF;"))
            return(-1);
        bsent=write(sock, file, sizeof(file));
        if(bsent < sizeof(file))
            return(-1);
        bsent=write(sock, ";" , sizeof(";"));
        if(bsent < sizeof(";"))
            return(-1);
        int tam= fileinfo.st_size;
        char str[20];
        sprintf(str,tam);
        bsent=write(sock, str , sizeof(str));
        if(bsent < sizeof(str))
            return(-1);
        bsent=write(sock, ";" , sizeof(";"));
        if(bsent < sizeof(";"))
            return(-1);
        /*enviar fichero a trozos*/
        /*
        if((fp = fopen(file,"r")) == NULL) // Abrir fichero
        {
            fprintf(stderr,"Error al abrir el fichero %s.\n",file);
            exit(1);
        }
        while((n=fread(buf,1,MAX_BUF,fp))==MAX_BUF) // Enviar fichero a trozos
            write(sock,buf,MAX_BUF);
        if(ferror(fp)!=0)
        {
            fprintf(stderr,"Error al enviar el fichero.\n");
            exit(1);
        }
        write(sock,buf,n); // Enviar el ultimo trozo de fichero
        
        return 0;
        
    }
}*/
int sendDelete(int sock, char * file)
{
    int bsent;
    bsent = write(sock, "DEL;", sizeof("DEL;"));
    if(bsent < sizeof("DEL;"))
        return(-1);
    
    bsent=write(sock, file, sizeof(file));
    if(bsent < sizeof(file))
        return(-1);
    
    return 0; 
}
int sendRename(int sock, char * file, char * file2)
{
    int bsent;
    bsent = write(sock, "REN;", sizeof("REN;"));
    if(bsent < sizeof("REN;"))
        return(-1);
    
    bsent=write(sock, file, sizeof(file));
    if(bsent < sizeof(file))
        return(-1);
    
    bsent=write(sock, ";" , sizeof(";"));
    if(bsent < sizeof(";"))
        return(-1);
    
    bsent=write(sock, file2, sizeof(file2));
    if(bsent < sizeof(file2))
        return(-1);
    return 0; 
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