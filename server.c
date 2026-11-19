#include <asm-generic/errno.h>
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
#include <aio.h>

#define MAXLINE 4096
#define SERV_PORT 3000

int main(int argc, char **argv) {
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	char buf[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	struct aiocb cbs[1024];
	memset(cbs, 0, sizeof(cbs));
	char cbf[1024] = {0};
	char cbbuf[1024][MAXLINE] = {0};
	int cnt = 0;

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
                        // Just trigger read here for async io.
                        printf("Triggering aio read.\n");
                        memset(&cbs[connfd], 0, sizeof(cbs[connfd]));
                        memset(&cbbuf[connfd], 0, sizeof(cbbuf[connfd]));
                        int rfd = open(buf, O_RDONLY);
                        memset(buf, 0, sizeof(buf));
                        cbs[connfd].aio_fildes = rfd;
                        cbs[connfd].aio_buf = cbbuf[connfd];
                        cbs[connfd].aio_nbytes = MAXLINE - 1;
                        cbs[connfd].aio_offset = 0;
                        cbf[connfd] = 1;
                        cnt++;

                        if (aio_read(&cbs[connfd]) == -1) {
                            perror("aio_read error\n");
                            cnt--;
                            cbf[connfd] = 0;
                            close(rfd);
                        }
                        FD_CLR(i, &mfds);
                    }
				}
			}
		}

		// After triggering read, now its time to check what all already triggered reads are going on. If any of them is done, send that data back to the client.
		if (cnt > 0) {
		    // printf("cnt = %d", cnt);
		    for (int i = 0; i < 1024; ++i) {
				if (cbf[i] == 1) {
				    if (aio_error(&cbs[i]) == 0) {
						printf("aio_error(&cbs[i]): %d\n", aio_error(&cbs[i]));
						aio_return(&cbs[i]);
						send(i, cbbuf[i], strlen(cbbuf[i]), 0);
						close(i);
						close(cbs[i].aio_fildes);
						// FD_CLR(i, &mfds);
						cnt--;
						cbf[i] = 0;
					}
				}
			}
		}
	}

	close(socket_fd);

	return 0;
}
