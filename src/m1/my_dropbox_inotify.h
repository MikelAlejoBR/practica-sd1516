#define EVBUFF_SIZE 4096
#define MAX_BUFF 1024

static int handle_events(int sock, int fd, int *wd, int wdlen, char* dir);
int inotify(char dir[]);