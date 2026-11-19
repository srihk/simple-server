#include <bits/types/struct_timeval.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>

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

	fd_set rfds, mfds;
	FD_ZERO(&mfds);
	FD_SET(socket_fd, &mfds);
	int mfd = socket_fd;
	for (;;) {
	    rfds = mfds;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		select(mfd + 1, &rfds, NULL, NULL, &tv);

		for (int i = 0; i < mfd + 1; ++i) {
		    if (FD_ISSET(i, &rfds)) {
				if (i == socket_fd) {
                    clilen = sizeof(cliaddr);
                    int connfd = accept(socket_fd, (struct sockaddr *) &cliaddr, &clilen);
                    FD_SET(connfd, &mfds);
                    if (mfd < connfd) {
                        mfd = connfd;
                    }
				} else {
				    int connfd = i;
                    read(connfd, buf, sizeof(buf));
    				printf("%s!\n", buf);
    				printf("Request received\n");
                    if (strcmp(buf, "time") == 0) {
                        time_t now = time(NULL);
        				char* nows = ctime(&now);
        				send(connfd, nows, strlen(nows), 0);
                    } else {
                        int rfd = open(buf, O_RDONLY);
                        printf("----->%s!\n", buf);
                        if (rfd == -1) {
                            printf("Couldn't open the file.\n");
                            strcpy(buf, "Couldn't open the file.\n");
                        } else {
                            read(rfd, buf, sizeof(buf));
                            close(rfd);
                        }

                        send(connfd, buf, strlen(buf), 0);
                        memset(buf, 0, sizeof(buf));
                    }
    				
    				close(connfd);
                    FD_CLR(connfd, &mfds);
				}
			}
		}
	}

	close(socket_fd);

	return 0;
}
