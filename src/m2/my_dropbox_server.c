#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

//Incluidos por Marcos
#include <sys/syscall.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <signal.h>
#include <time.h>

#define MAX_BUF 1024
#define PORT 6012
#define FILENAME  "./MiNube/"

int main(int arhc, char *argv[])
{
	int sock, n1, n2, usuario, comando, dialogo;   //socket, numero de bytes, numero de usuario, numero de comando, nuevo socket
	struct sockaddr_in dir_serv, dir_cli;
	char buf[MAX_BUF];
	socklen_t tam_dir;

	// Crear el socket
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
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
	
	// Ignorar la senal enviada cuando un proceso hijo acaba, para que no quede como zombi
	signal(SIGCHLD, SIG_IGN);
		
	// Tamaño de la direccion IP del cliente
	tam_dir = sizeof(dir_cli);
	
	while(1){
		//PETICION DE CONEXION DE CLIENTE
		n1=recvfrom(sock, buf, MAX_BUF, 0, (struct sockaddr *) &dir_cli, &tam_dir);
		if(n > 3 || !strcmp(buf, comandos[0]))
		{
			perror("Error al recibir el mensaje de conexion");
			exit(1);
		}	

		// Solicitamos conexion de cliente
		if(connect(sock, (struct sockaddr *) &dir_cli, tam_dir) < 0)
		{
			perror("Error al conectar el socket TCP con el del cliente/r/n");
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
	int n, rc;  // numero de bytes
	char buf[MAX_BUF], error[MAX_BUF], nombre[MAX_BUF];
	
	int file_size;
    FILE *received_file;
    int dat_f = 0;
	ssize_t len;

	// Establecer estado como inicial
	estado = ST_INIT;

	// Leer lo enviado por el cliente
	if((n=read(sock,buf,MAX_BUF)) <= 0)
		return;

	// Comprobar si el comando es conocido
	if((comando=busca_substring(buf,comandos)) < 0)
	{
		sprintf(error,"%s:1\n",comandos[4]);
		write(sock,error,6);
	}
	
	// Realizar la operacion correspondiente segun el comando recibido
	switch(comando)
	{
		case COM_TRF:
			if(estado != ST_INIT) // Comprobar si el estado es el esperado
			{
				printf("Error de estado/n");
				exit(1);
			}
			buf[n] = 0;	// Borrar EOL
			
			estado = COM_TRF;

			/* GUARDAR NOMBRE DE COPIA DE FICHERO PREVIO AL BORRADO */
			sprintf(nombre,"s%s",FILENAME,buf+4);
			sprintf(nombre_copia,"s%s_minubeCOPY",FILENAME,buf+4);
			
			sprintf(ack,"%s\n",comandos[3]);
			write(sock, ack, 4); //ACK DE CONFIRMACION
			
			/* REALIZAR LA COPIA DEL FICHERO CON EL COMANDO cp */
			sprintf(comando,"cp %s %s",nombre,nombre_copia);
			system(comando);
			
			remove(nombre);
			
			//http://stackoverflow.com/questions/11952898/c-send-and-receive-file
			/* RECIBIR TAMAÑO DEL FICHERO*/
			recv(sock, buf, MAX_BUF, 0);
			tam_file = atoi(buffer);

			/* CREACIÓN DEL FICHERO ACTUALIZADO */
			rec_file = fopen(nombre, "w");

			/* RECEPCIÓN DEL CONTENIDO DEL FICHERO */
			while (((parte = recv(sock, buf, MAX_BUF, 0)) > 0) && (tam_file > 0))
			{
				fwrite(buf, sizeof(char), parte, rec_file);
				tam_file -= parte;
				//fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", parte, tam_file);
			}
			
			/* CIERRE DEL FICHERO */
			fclose(rec_file);
			
			remove(nombre_copia);
			
			sprintf(ack,"%s\n",comandos[3]);
			write(sock, ack, 4); //ACK DE CONFIRMACION
			
			break; //se termina la conexion
			
		case COM_DEL:
			if(estado != ST_INIT) // Comprobar si el estado es el esperado
			{
				printf("Error de estado/n");
				exit(1);
			}
			
			buf[n] = 0;	// Borrar EOL
			estado = COM_DEL;
			
			sprintf(nombre,"s%s",FILENAME,buf+4);

			remove(nombre);
			
			sprintf(ack,"%s\n",comandos[3]);
			write(sock, ack, 4); //ACK DE CONFIRMACION
			
			break; //se termina la conexion
			
		case COM_FIN:
			if(estado != COM_TRF || estado != COM_DEL)
			{
				printf("Error de estado/n");
				exit(1);
			}
			return;
	}
}

/*
* Busca el string 'string' en el array de strings 'strings'. El último elemento del array 'strings' ha de ser NULL.
* Devuelve el índice de la primera aparición de 'string' en 'strings'. Si 'string' no esta en el array, devuelve un valor negativo.
*/

int busca_string(char *string, char **strings)
{
	int i=0;
	while(strings[i] != NULL)
	{
		if(!strcmp(string,strings[i]))
			return i;
		i++;
	}
	return -1;
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
