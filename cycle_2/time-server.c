#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#define PORT 8080
#define MAXLINE 1024
int main() {
int sockfd,nclient=1;
char buffer[MAXLINE];
struct sockaddr_in servaddr, cliaddr;
if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
perror("socket creation failed");
exit(EXIT_FAILURE);
}
memset(&servaddr, 0, sizeof(servaddr));
memset(&cliaddr, 0, sizeof(cliaddr));
servaddr.sin_family = AF_INET; // IPv4
servaddr.sin_addr.s_addr = INADDR_ANY;
servaddr.sin_port = htons(PORT);
if ( bind(sockfd, (const struct sockaddr *)&servaddr,
sizeof(servaddr)) < 0 )
{
perror("bind failed");
exit(EXIT_FAILURE);
}
int len, n;
len = sizeof(cliaddr);
while(1)
{ n = recvfrom(sockfd, (char *)buffer, MAXLINE,
MSG_WAITALL, ( struct sockaddr *) &cliaddr,
&len);
time_t currentTime;
time(&currentTime);
printf("Client %d requested for time at %s", nclient, ctime(&currentTime));
sendto(sockfd, ctime(&currentTime), 30,
MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
len);
nclient++;
}
return 0;
}
