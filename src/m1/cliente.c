#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "cliente.h"
#include "protocolo.h"
#include "inotify.h"

int main(int argc, char *argv[]) {
	
	if (argc > 1) {
		fprintf(stderr, "El programa ha de ejecutarse sin parámetros. Uso: %s", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	char directorio[] = 
		"/home/USR/practica-sd1516/src/m1/testdir";
	inotify(directorio);
	
	exit(EXIT_SUCCESS);
}