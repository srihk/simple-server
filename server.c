#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#define MAXLINE 4096
#define SERV_PORT 3000

int main(int argc, char **argv) {
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	char buf[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	if (socket_fd == -1) {
		printf("Couldn't open a socket.\n");
		exit(3);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	
	if (bind(socket_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
		printf("Couldn't bind the socket with fd: %d\n", socket_fd);
		exit(4);
	}

	listen(socket_fd, 8);

	printf("Server is running. Waiting for connections.\n");

	for (;;) {
		clilen = sizeof(cliaddr);
		int connfd = accept(socket_fd, (struct sockaddr *) &cliaddr, &clilen);
		printf("Request received\n");
		time_t now = time(NULL);
		char* nows = ctime(&now);
		send(connfd, nows, strlen(nows), 0);
		close(connfd);
	}

	close(socket_fd);

	return 0;
}
