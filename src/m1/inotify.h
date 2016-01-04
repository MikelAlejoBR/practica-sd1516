#define EVBUFF_SIZE 4096

static int handle_events(int fd, int *wd, int wdlen, char* dir);
int inotify(char dir[]);