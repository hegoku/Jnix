#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT     8080
#define MAXLINE 1024

 int main() {
int sockfd;
char buffer[1024];
//char *hello = "Hello from client";
char *hello = "fds#";
struct sockaddr_in     servaddr;

if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
}

memset(&servaddr, 0, sizeof(servaddr));

servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(4444);
servaddr.sin_addr.s_addr = INADDR_ANY;
servaddr.sin_addr.s_addr = htonl(0x7F000001);;

int n, len;

sendto(sockfd, (const char *)hello, strlen(hello),  0, (const struct sockaddr *) &servaddr,  sizeof(servaddr));
printf("Hello message sent.\n");

n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
buffer[n] = '\0';
printf("Server : %s\n", buffer);

close(sockfd);
return 0;
 }