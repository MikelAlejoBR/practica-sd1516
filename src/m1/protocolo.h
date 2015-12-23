#define PORT 50000

int sendack(int sock);
int senderr(int sock, int ecode);
int sendfin(int sock);
int getsockfd();
int releasesockfd(int sock);