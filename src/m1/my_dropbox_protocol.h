#define PORT 6012
#define TIMEOUT 10;
#define MAX_BUF 1024

int waitforack(int sock);
int sendConnect(int sock);
int sendack(int sock);
int senderr(int sock, int ecode);
int sendFile(int sock, char * file);
int sendfin(int sock);
int getsockfd();
int releasesockfd(int sock);