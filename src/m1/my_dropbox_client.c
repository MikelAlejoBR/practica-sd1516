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
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
int main(int argc, char *argv[]) {
    
    if (argc > 1) {
        fprintf(stderr, "El programa ha de ejecutarse sin parámetros. Uso: %s", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    struct passwd *pw = getpwuid(getuid());
    
    const char *homedir = pw->pw_dir;
    char directorio[80];
    strcpy(directorio,homedir);
    
    strcat(directorio,"/testdir");
    
    
    /* Directorio a monitorizar */
    inotify(directorio);
    
    exit(EXIT_SUCCESS);
}