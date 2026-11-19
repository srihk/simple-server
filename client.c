#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERV_PORT 3000

int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in servaddr;
	char sendline[4096], recvline[4096];

	if (argc < 2 || argc > 3) {
		perror("Usage: TCPClient <IP address of the server> <path>\n");
		exit(1);
	}

	if (argc == 3) {
	    printf("%s!!", argv[2]);
		strcpy(sendline, argv[2]);
	} else {
	    strcpy(sendline, "time");
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Problem in creating the socket\n");
		exit(2);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(SERV_PORT);

	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror("Problem in connecting to the server\n");
		exit(3);
	}

	send(sockfd, sendline, strlen(sendline), 0);

	if (recv(sockfd, recvline, 4096, 0) == 0) {
		perror("The server terminated prematurely");
		exit(4);
	}

	printf("%s", "String received from the server: ");
	fputs(recvline, stdout);

	exit(0);
}
