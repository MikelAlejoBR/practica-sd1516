#include <stdio.h>
#include <stdlib.h>

#include "cliente.h"
#include "protocolo.h"

int main(int argc, char *argv[]) {
	if (argc > 1) {
		fprintf(stderr, "El programa ha de ejecutarse sin parámetros. Uso: %s", argv[0]);
		exit(EXIT_FAILURE);
	}

	exit(EXIT_FAILURE);
}