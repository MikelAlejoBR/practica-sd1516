#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "my_dropbox_client.h"
//#include "my_dropbox_protocol.h"
#include "my_dropbox_inotify.h"


int main(int argc, char *argv[]) {
          
          if (argc > 1) {
	    fprintf(stderr, "El programa ha de ejecutarse sin parámetros. Uso: %s", argv[0]);
	    exit(EXIT_FAILURE);
          }
          
          /* Directorio a monitorizar */
          char directorio[] = "/home/olivia/testdir";
          inotify(directorio);
          
          exit(EXIT_SUCCESS);
}