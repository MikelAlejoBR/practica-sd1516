#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include "my_dropbox_server.h"
struct stat fileinfo;
#define MAX_BUF 1024
#define TIMEOUT 10;
char buf[MAX_BUF];
FILE *fp;

char ack[4];

int main(int arhc, char *argv[])
{
	int sock, n, s_up;   //socket de escucha, numero de bytes, socket particular al cliente (funcion update)
	struct sockaddr_in dir_serv, dir_cli;
	char buf[MAX_BUF];
	socklen_t sin_size, tam_dir;
	
	struct timeval timeout;      
    timeout.tv_sec = 40;
    timeout.tv_usec = 0;

	// Crear el socket
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Error al crear el socket");
		exit(1);
	}
	
	memset(&dir_serv, 0, sizeof(dir_serv));
	dir_serv.sin_family = AF_INET;
	dir_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	dir_serv.sin_port = htons(PORT);
	
	// Asignar direccion al socket
	if(bind(sock, (struct sockaddr *) &dir_serv, sizeof(dir_serv)) < 0)
	{
		perror("Error al asignarle una direccion al socket");
		exit(1);
	}
	
	// Tamaño de la direccion IP del cliente
	tam_dir = sizeof(dir_cli);
	
	//Nos quedamos a la escucha de una conexion
	listen(sock, 1);
	sin_size = sizeof(struct sockaddr_in);
	
	while(1){

		// Aceptamos una conexión entrante
		if ((s_up = accept(sock,(struct sockaddr *)&dir_cli,&sin_size)) < 0)
			return 0;
		
		// Añadimos timeouts en las operaciones de entrada y salida del socket
		if(setsockopt (s_up, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		{
			close(s_up);
		}	
	
		if(setsockopt (s_up, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		{
			close(s_up);
			exit(1);
		}

		//PETICION DE CONEXION DE CLIENTE (Comando CON)
		if ((n = read(s_up,buf,MAX_BUF)) < 0 ){
			perror("Error al recibir la peticion de conexion del cliente");
		}

		if(n != 4 || strncmp(buf, comandos[0], 3) != 0)
		{
			perror("El mensaje recibido no es el de conexion [CON].");
		}else
			printf("Recibido comando: %s | %s\n", buf, obtenerTiempo());
		
		// Construir ACK
		if (sprintf(ack,"%s",comandos[3]) < 0)
			perror("sprintf ");
		if (write(s_up, ack, 4) < 0)
			perror("write 1 "); // enviar ACK CONFIRMACION
		else
			printf("Enviado comando: %s | %s\n", ack, obtenerTiempo());
	
		// Iniciamos la actualizacion
		update(s_up);

		// Recepción comando FIN
		if ((n = read(s_up,buf,MAX_BUF)) < 0 ){
			perror("Error al recibir la peticion de desconexion del cliente");
		}
		if(n != 4 || strncmp(buf, comandos[5], 3) != 0)
		{
			perror("El mensaje recibido no es el de conexion [FIN].");
		}else
			printf("Recibido comando: %s | %s\n", buf, obtenerTiempo());

		// Cerramos socket de cliente
		if(close(s_up) != -1)
			printf("Cerrado socket cliente | %s\n", obtenerTiempo());
		else
			perror("Error: socket de cliente no cerrado.");
	}
	close(sock);
}

void update(int sock)
{
	int leido = 0;
	int comando, tam_file, i, n;
	char buf[MAX_BUF], temp[MAX_BUF], nombre[MAX_BUF], nombre_copia[MAX_BUF], nombre_nuevo[MAX_BUF], cmd[MAX_BUF];
	
    FILE *rec_file;
	ssize_t parte;

	// Leer lo enviado por el cliente
	if((n=read(sock,buf,MAX_BUF)) < 0)
		perror("ERROR al leer el comando enviado por el cliente.");	
	else
		printf("Recibido comando: %s | %s\n", buf, obtenerTiempo());
	// Comprobar si el comando es conocido
	if((comando=busca_substring(buf,comandos)) < 0)
		perror("Encontrar el comando");
	
	// Realizar la operacion correspondiente segun el comando recibido
	switch(comando)
	{
		case COM_TRF:
			buf[n] = 0;	// Borrar EOL

			/* GUARDAR NOMBRE DE COPIA DE FICHERO PREVIO AL BORRADO */
			sprintf(nombre,"%s%s",FILENAME,buf+4);
			//sprintf(nombre,"%s",buf+4);
			
			sprintf(nombre_copia,"%s_minubeCOPY",buf+4);
			sprintf(nombre_copia,"%s%s_minubeCOPY",FILENAME,buf+4);

			/* REALIZAR LA COPIA DEL FICHERO CON EL COMANDO cp */
			sprintf(cmd,"cp %s %s",nombre,nombre_copia);
			system(cmd);
			printf("Ejecutada llamada al sistema: %s | %s\n", cmd, obtenerTiempo());
			
			if (remove(nombre) > 0)
				printf("Realizada operacion: remove(%s) | %s\n", nombre, obtenerTiempo());
			
			i = posc(buf+4,';');

			tam_file = atoi(buf+4+i);

			/* CREACIÓN DEL FICHERO ACTUALIZADO */
			rec_file = fopen(nombre, "w");
			
			sprintf(ack,"%s",comandos[3]);
			write(sock, ack, 4); //ACK DE CONFIRMACION
			printf("Enviado comando: %s | %s\n", ack, obtenerTiempo());

			/* RECEPCIÓN DEL CONTENIDO DEL FICHERO */
			while (((parte = recv(sock, buf, MAX_BUF, 0)) > 0) && (tam_file > 0))
			{
				fwrite(buf, sizeof(char), parte, rec_file);
				tam_file -= parte;
			}
			
			if(tam_file != 0)
				return;	
			
			/* CIERRE DEL FICHERO */
			fclose(rec_file);

			
			remove(nombre_copia);
			
			sprintf(ack,"%s",comandos[3]);
			write(sock, ack, 4); //ACK DE CONFIRMACION
			
			backup(nombre, COM_TRF);

			return; //se termina la conexion
			
		case COM_DEL:	
			buf[n] = 0;	// Borrar EOL
	
			sprintf(nombre,"%s%s",FILENAME,buf+4);

			if(remove(nombre) != -1)
				printf("Realizada operacion: remove('%s') | %s\n", nombre, obtenerTiempo());
			
			sprintf(ack,"%s",comandos[3]);
			
			if (write(sock, ack, 4) > 0) //ACK DE CONFIRMACION
				printf("Enviado comando: %s | %s\n", ack, obtenerTiempo());

			backup(nombre, COM_DEL);

			return; //se termina la conexion
			
		case COM_REN:	
			buf[n] = 0;	// Borrar EOL
			i = posc(buf+4,';');

			strncpy(temp, buf+4,i-1);
			temp[i-1] = 0;
			sprintf(nombre,"%s%s",FILENAME,temp);

			sprintf(nombre_nuevo,"%s%s",FILENAME,buf+i+4);

			sprintf(cmd,"mv %s %s",nombre,nombre_nuevo);
			
			if (system(cmd) != -1)
				printf("Realizada operacion: %s | %s\n", cmd, obtenerTiempo());
			
			sprintf(ack,"%s",comandos[3]);
			if(write(sock, ack, 4) > 0) //ACK DE CONFIRMACION
				printf("Enviado comando: %s | %s\n", ack, obtenerTiempo());

			backup(nombre, COM_REN);
			return; //se termina la conexion	
	}
}

/*
* Funcion que devuelve la ultima posicion del elemento ";" dado una cadena de caracteres
*/
int posc( char cad[], char c)
{
   int pos = -1;
   int len = strlen( cad);
   int i;

   for(i = 0; /*pos == -1 && */i < len; i++){ // si quitas la condición pos == -1
            // te devuelve la última posición encontrada (si es que hay más de 1)
      if(*(cad+i) == c)
         pos = i+1;
   }
   return pos;
}

/*
* Busca si el string 'string' comienza con algún string contenido en el array de strings 'strings'. El último elemento del array 'strings' ha de ser NULL.
* Devuelve el índice del primer elemento de 'strings' que coincide con el comienzo de 'string'. Si no hay coincidencia, devuelve un valor negativo.
*/
int busca_substring(char *string, char **strings)
{
	int i=0;
	while(strings[i] != NULL)
	{
		if(!strncmp(string,strings[i],strlen(strings[i])))
			return i;
		i++;
	}
	return -1;
}

char * obtenerTiempo(){
	time_t t;
	time(&t);
	return asctime(localtime(&t));
}

void callback(){}

int backup(char *path, int comando){

/* CREAR SOCKET */
int sock = getsockfd();
	if (sock < 0)
		return(-1);
	
	if (sendConnect(sock))
		return(-1);

switch(comando)
{
	case COM_TRF:
	sendFile(sock , &path);
	return;
	case COM_REN:
		// Renombrar fichero no esta en cliente
	return;
	case COM_DEL:
		// Eliminar fichero
	return;
}

releasesockfd(sock);
}

int sendFile(int sock, char *path){
	int bsent;
	int n;
	int tam = fileinfo.st_size;
	char buff[MAX_BUF];
	
	if(stat(path, &fileinfo)<0)
		fprintf(stderr,"Fichero no encontrado\n");
	else {
	
		snprintf(buff, sizeof(buff), "TRF;%s;%d", path, tam);
		bsent=write(sock, buff, strlen(buff));
		
		if(bsent < strlen(buff))
			return(-1);

		if (waitforack(sock) != 0)
			return(-1);
		
		printf("ACK OKey!");
		
		/*enviar fichero a trozos*/
		if((fp = fopen(path,"r")) == NULL) // Abrir fichero
		{
			fprintf(stderr,"Error al abrir el fichero %s.\n",path);
			exit(1);
		}
		
		/* Enviar fichero a trozos */
		while((n=fread(buf,1,MAX_BUF,fp))==MAX_BUF)
			write(sock,buf,MAX_BUF);
		if(ferror(fp)!=0)
		{
			fprintf(stderr,"Error al enviar el fichero.\n");
			exit(1);
		}
		write(sock,buf,n); // Enviar el ultimo trozo de fichero
		
		return 0;
		
	}
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
	
	if(connect(sock, (struct sockaddr *) &serv_info, sizeof(serv_info)) 
		< 0) {
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

int sendConnect(int sock) {
	int bsent;
	bsent = write(sock, "CON", sizeof("CON"));
	if (bsent < sizeof("CON"))
		return(-1);
	return(0);
}

int waitforack(int sock) {
	char buffer[6];
	
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
	else
		return(0);
}
