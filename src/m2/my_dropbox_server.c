#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>

//Incluidos por Marcos
#include <sys/syscall.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <signal.h>
#include "my_dropbox_server.h"

char ack[4];

int main(int arhc, char *argv[])
{
	int sock, n, l;   //socket, numero de bytes, numero de usuario, numero de comando, nuevo socket
	struct sockaddr_in dir_serv, dir_cli;
	char buf[MAX_BUF];
	socklen_t sin_size, tam_dir;
	
	struct timeval timeout;      
    timeout.tv_sec = 10;
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
	
	// Añadimos timeouts en las operaciones de entrada y salida del socket
	if(setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
	{
		close(sock);
		exit(1);
	}	
	
	if(setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
	{
		close(sock);
		exit(1);
	}
	
	//Nos quedamos a la escucha de una conexion
	listen(sock, 1);
	sin_size = sizeof(struct sockaddr_in);
	
	while(1){
		if ((l = accept(sock,(struct sockaddr *)&dir_cli,&sin_size)) < 0)
			return 0;
			
		//PETICION DE CONEXION DE CLIENTE
		//n=recvfrom(sock, buf, MAX_BUF, 0, (struct sockaddr *) &dir_cli, &tam_dir);
		n = read(sock,buf,MAX_BUF);
		if(n > 3 || !strcmp(buf, comandos[0]))
		{
			perror("Error al recibir el mensaje de conexion");
			exit(1);
		}	

		sprintf(ack,"%s\n",comandos[3]);
		write(sock, ack, 4); //ACK DE CONFIRMACION

		// Iniciamos la actualizacion
		update(sock);	
		close(sock);
		//exit(0);
	}
}

void update(sock)
{
	int leido = 0;
	int comando, tam_file;
	int n;  // numero de bytes
	char buf[MAX_BUF], nombre[MAX_BUF], nombre_copia[MAX_BUF];
	char * cmd;
	
    FILE *rec_file;
	ssize_t parte;

	// Leer lo enviado por el cliente
	if((n=read(sock,buf,MAX_BUF)) < 0)
		return;	
	
	// Comprobar si el comando es conocido
	if((comando=busca_substring(buf,comandos)) < 0)
		return;
	
	// Realizar la operacion correspondiente segun el comando recibido
	switch(comando)
	{
		case COM_TRF:
			buf[n] = 0;	// Borrar EOL

			/* GUARDAR NOMBRE DE COPIA DE FICHERO PREVIO AL BORRADO */
			sprintf(nombre,"%s%s",FILENAME,buf+4);
			sprintf(nombre_copia,"%s%s_minubeCOPY",FILENAME,buf+4);
			
			sprintf(ack,"%s\n",comandos[3]);
			write(sock, ack, 4); //ACK DE CONFIRMACION
			
			/* REALIZAR LA COPIA DEL FICHERO CON EL COMANDO cp */
			sprintf(cmd,"cp %s %s",nombre,nombre_copia);
			system(cmd);
			
			remove(nombre);
			
			//http://stackoverflow.com/questions/11952898/c-send-and-receive-file
			/* RECIBIR TAMAÑO DEL FICHERO*/
			if((n=recv(sock, buf, MAX_BUF, 0)) < 0)
				return;

			//tam_file = atoi(buffer);
			tam_file = atoi(buf);

			/* CREACIÓN DEL FICHERO ACTUALIZADO */
			rec_file = fopen(nombre, "w");

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
			
			sprintf(ack,"%s\n",comandos[3]);
			write(sock, ack, 4); //ACK DE CONFIRMACION
			
			return; //se termina la conexion
			
		case COM_DEL:	
			buf[n] = 0;	// Borrar EOL
			
			sprintf(nombre,"%s%s",FILENAME,buf+4);

			remove(nombre);
			
			sprintf(ack,"%s\n",comandos[3]);
			write(sock, ack, 4); //ACK DE CONFIRMACION
			
			return; //se termina la conexion			
	}
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

void callback(){}

void backup(){}

void heartbeat(){}
