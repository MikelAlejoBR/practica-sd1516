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

#include "my_dropbox_protocol.h"

struct stat fileinfo;
char buf[MAX_BUF];
char str[MAX_BUF];
FILE *fp;

/*
 * waitforack
 * @param int sock
 * 
 * Waits for an ACK message from the server
 * 
 * @return -1 if something different from ACK was received
 * @return 0 if ACK was received
 * @return 1 if TIMEOUT was reached
 */
int waitforack(int sock) {
          char buffer[6];
          buffer[3]='\0';
          
          fd_set set;
          struct timeval timeout;
          
          FD_ZERO(&set);
          FD_SET(sock, &set);
          
          timeout.tv_sec = TIMEOUT;
          timeout.tv_usec = 0;
          
          if (!select(FD_SETSIZE, &set, NULL, NULL, &timeout))
	    return(1);
          
          read(sock, buffer, sizeof(buffer));
          if (strcmp("ACK", buffer))
	    return(-1);
          else{
	    return(0);
          }
}

/* 
 * sendConnect
 * @param int sock
 * 
 * Sends the command "CON" to establish a connection with the server
 * 
 * @return -1 if something different from the command "CON" is sent
 * @return 0 if ACK is received
 * @return 1 if ACK is not received
 */
int sendConnect(int sock) {
          int bsent;
          bsent = write(sock, "CON", sizeof("CON"));
          if (bsent < sizeof("CON"))
	    return(-1);
          
          if (waitforack(sock) == 0)
	    return 0;
          else
	    return 1;
}


/* 
 * sendack
 * @param int sock
 * 
 * Sends an ACK message to the server 
 * 
 * @return -1 if something different from the command "ACK" is sent
 * @return 0 if ACK is correctly sent
 */
int sendack(int sock) {
          int bsent;
          bsent = write(sock, "ACK", sizeof("ACK"));
          if(bsent < sizeof("ACK"))
	    return(-1);
          return 0;
}

/* 
 * senderr
 * @param int sock
 * @param int ecode
 * 
 * Sends an error message to the server using the command "ERR"
 * 
 * @return -1 if something different from the command "ACK" is sent
 * @return 0 if the error message is correctly sent
 */
int senderr(int sock, int ecode) {
          char errbuff[7];
          snprintf(errbuff, sizeof(errbuff), "ERR %d", ecode);
          
          int bsent;
          bsent = write(sock, errbuff, sizeof(errbuff));
          
          if(bsent < sizeof(errbuff))
	    return(-1);
          return 0;
}

/* 
 * sendFile
 * @param int sock
 * @param char *path
 * 
 * Sends a file to the server using the command "TRF"
 * 
 * @return -1 if something different from the command "ACK" is sent
 * @return 0 ACK is correctly sent
 */
int sendFile(int sock, char *path, char *dir){
          int bsent;
          int n;
          int tam = fileinfo.st_size;
          
          char  completo[80];
          
          printf("%s\n", dir);
          strcpy(completo,dir);
          strcat(completo,"/");
          strcat(completo,path);
          
          if(stat(completo, &fileinfo)<0)
          {
	    fprintf(stderr,"Fichero no encontrado\n");
	    return(-1);
          }
          
          else
          {
	    sprintf(buf, "TRF;%s;%ld", path, tam);
	    bsent=write(sock, buf, strlen(buf));
	    
	    if(bsent < strlen(buf))
	              return(-1);
	    
	    if (!S_ISDIR(fileinfo.st_mode)) // Si no es un directorio
	    {
	              /*enviar fichero a trozos*/
	              if((fp = fopen(completo,"r")) == NULL) // Abrir fichero
	              {
		        fprintf(stderr,"Error al abrir el fichero %s.\n",path);
		        exit(1);
	              }
	              while((n=fread(buf,1,MAX_BUF,fp))==MAX_BUF) // Enviar fichero		
		        //a trozos
		        write(sock,buf,MAX_BUF);
	              if(ferror(fp)!=0)
	              {
		        fprintf(stderr,"Error al enviar el fichero.\n");
		        exit(1);
	              }
	              write(sock,buf,n); // Enviar el ultimo trozo de fichero
	    }
	    if (waitforack(sock) != 0) {
	              return(-1);
	    }
	    
	    sendfin(sock);
	    return 0;
          }
}

/* 
 * sendDelete
 * @param int sock
 * @param char *path
 * 
 * Sends the command "DEL" to the server so that it removes the file specified in "path"
 * 
 * @return -1 if something different from the command "DEL" is sent
 * @return 0 the command "DEL" is correctly sent
 */
int sendDelete(int sock, char * path)
{
          int bsent;
          
          sprintf(buf, "DEL;%s", path);
          bsent = write(sock, buf, strlen(buf));
          
          if(bsent < strlen(buf))
	    return(-1);
          
          if (waitforack(sock) != 0) {
	    return(-1);
          }
          
          sendfin(sock);
          
          return 0; 
}

/* 
 * sendRename
 * @param int sock
 * @param char *path1
 * @param char *path2
 * 
 * Sends the command "REN" to the server so that it renames the file specified in "path1"
 * 
 * @return -1 if something different from the command "REN" is sent
 * @return 0 the command "REN" is correctly sent
 */
int sendRename(int sock, char * path1, char * path2)
{
          int bsent;
          
          sprintf(buf, "REN;%s;%s", path1, path2);
          bsent = write(sock, buf, strlen(buf));
          
          if(bsent < strlen(buf))
	    return(-1);
          
          if (waitforack(sock) != 0) {
	    return(-1);
          }
          
          sendfin(sock);
          
          return 0; 
}

/* 
 * sendfin
 * @param int sock
 * 
 * Sends the command "FIN" to the server to end the connection
 * 
 * @return -1 if something different from the command "FIN" is sent
 * @return 0 the command "FIN" is correctly sent
 */
int sendfin(int sock) {
          int bsent;
          bsent = write(sock, "FIN", sizeof("FIN"));
          if(bsent < sizeof("FIN"))
	    return(-1);
          
          return waitforack(sock);
}

/* 
 * getsockfd
 * 
 * Creates a new socket in order to establish a connection with the server 
 * 
 * @return -1 if an error occurs during the creation of the socket or the establishment of the connection
 * @return 0 if the socket is correctly created and the connection is successfully established
 */
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

/* 
 * releasesockfd
 * @param int sock
 * 
 * Closes the socket specified
 * 
 * @return -1 if an error occurs during the closing
 * @return 0 if the socket is correctly closed
 */
int releasesockfd(int sock) {
          if(close(sock) < 0) {
	    fprintf(stderr, "Error al cerrar el socket");
	    return(-1);
          }
          return 0;
}