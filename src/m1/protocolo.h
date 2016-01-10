#define PORT 50000
#define TIMEOUT 10;

int waitforack(int sock);
int sendack(int sock);
int senderr(int sock, int ecode);
int sendfin(int sock);
int getsockfd();
int releasesockfd(int sock);