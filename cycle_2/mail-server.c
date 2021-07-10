





























#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>



#define PORT		"2525"


#define DOMAIN		"kwf.dyndns.org"

#define BACKLOG_MAX	(10)
#define BUF_SIZE	4096
#define STREQU(a,b)	(strcmp(a, b) == 0)


struct int_ll {
	int d;
	struct int_ll *next;
};


struct {
	struct int_ll *sockfds;
	int sockfd_max;
	char *domain;
	pthread_t thread; 
} state;


void init_socket(void);
void *handle_smtp (void *thread_arg);
void *get_in_addr(struct sockaddr *sa);










int main (int argc, char *argv[]) {
	int rc, i, j;
	char strbuf[INET6_ADDRSTRLEN];

	
	char *syslog_buf = (char*) malloc(1024);
	sprintf(syslog_buf, "%s", argv[0]);
	openlog(syslog_buf, LOG_PERROR | LOG_PID, LOG_USER);

	
	state.domain = DOMAIN;

	
	init_socket();

	
	
	while (1) {
		fd_set sockets;
		FD_ZERO(&sockets);
		struct int_ll *p;

		for (p = state.sockfds; p != NULL; p = p->next) {
			FD_SET(p->d, &sockets);
		}

		
		select (state.sockfd_max+1, &sockets, NULL, NULL, NULL);

		
		for (p = state.sockfds; p != NULL; p = p->next) {
			if (FD_ISSET(p->d, &sockets)) {
				struct sockaddr_storage client_addr;
				socklen_t sin_size = sizeof(client_addr);
				int new_sock = accept (p->d, \
						(struct sockaddr*) &client_addr, &sin_size);
				if (new_sock == -1) {
					syslog(LOG_ERR, "Accepting client connection failed");
					continue;
				}

				
				void *client_ip = get_in_addr(\
						(struct sockaddr *)&client_addr);
				inet_ntop(client_addr.ss_family, \
						client_ip, strbuf, sizeof(strbuf));
				syslog(LOG_DEBUG, "Connection from %s", strbuf);

				
				
				int * thread_arg = (int*) malloc(sizeof(int));
				*thread_arg = new_sock;

				
				pthread_create(&(state.thread), NULL, \
						handle_smtp, thread_arg);

			}
		}
	} 

	return 0;
}















void init_socket(void) {
	int rc, i, j, yes = 1;
	int sockfd;
	struct addrinfo hints, *hostinfo, *p;

	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	state.sockfds = NULL;
	state.sockfd_max = 0;

	rc = getaddrinfo(NULL, PORT, &hints, &hostinfo);
	if (rc != 0) {
		syslog(LOG_ERR, "Failed to get host addr info");
		exit(EXIT_FAILURE);
	}

	for (p=hostinfo; p != NULL; p = p->ai_next) {
		void *addr;
		char ipstr[INET6_ADDRSTRLEN];
		if (p->ai_family == AF_INET) {
			addr = &((struct sockaddr_in*)p->ai_addr)->sin_addr; 
		} else {
			addr = &((struct sockaddr_in6*)p->ai_addr)->sin6_addr; 
		}
		inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));

		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sockfd == -1) {
			syslog(LOG_NOTICE, "Failed to create IPv%d socket", \
					(p->ai_family == AF_INET) ? 4 : 6 );
			continue;
		}

		setsockopt(sockfd, SOL_SOCKET, \
				SO_REUSEADDR, &yes, sizeof(int));

		rc = bind(sockfd, p->ai_addr, p->ai_addrlen);
		if (rc == -1) {
			close (sockfd);
			syslog(LOG_NOTICE, "Failed to bind to IPv%d socket", \
					(p->ai_family == AF_INET) ? 4 : 6 );
			continue;
		}

		rc = listen(sockfd, BACKLOG_MAX);
		if (rc == -1) {
			syslog(LOG_NOTICE, "Failed to listen to IPv%d socket", \
					(p->ai_family == AF_INET) ? 4 : 6 );
			exit(EXIT_FAILURE);
		}

		
		(sockfd > state.sockfd_max) ? (state.sockfd_max = sockfd) : 1;

		
		struct int_ll *new_sockfd = malloc(sizeof(struct int_ll));
		new_sockfd->d = sockfd;
		new_sockfd->next = state.sockfds;
		state.sockfds = new_sockfd;
	}

	if (state.sockfds == NULL) {
		syslog(LOG_ERR, "Completely failed to bind to any sockets");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(hostinfo);

	return;
}













void *handle_smtp (void *thread_arg) {
	syslog(LOG_DEBUG, "Starting thread for socket #%d", *(int*)thread_arg);

	int rc, i, j;
	char buffer[BUF_SIZE], bufferout[BUF_SIZE];
	int buffer_offset = 0;
	buffer[BUF_SIZE-1] = '\0';

	
	int sockfd = *(int*)thread_arg;
	free(thread_arg);

	
	int inmessage = 0;

	sprintf(bufferout, "220 %s SMTP CCSMTP\r\n", state.domain);
	printf("%s", bufferout);
	send(sockfd, bufferout, strlen(bufferout), 0);

	while (1) {
		fd_set sockset;
		struct timeval tv;

		FD_ZERO(&sockset);
		FD_SET(sockfd, &sockset);
		tv.tv_sec = 120; 
		tv.tv_usec = 0;

		
		select(sockfd+1, &sockset, NULL, NULL, &tv);

		if (!FD_ISSET(sockfd, &sockset)) {
			syslog(LOG_DEBUG, "%d: Socket timed out", sockfd);
			break;
		}

		int buffer_left = BUF_SIZE - buffer_offset - 1;
		if (buffer_left == 0) {
			syslog(LOG_DEBUG, "%d: Command line too long", sockfd);
			sprintf(bufferout, "500 Too long\r\n");
			printf("S%d: %s", sockfd, bufferout);
			send(sockfd, bufferout, strlen(bufferout), 0);
			buffer_offset = 0;
			continue;
		}

		rc = recv(sockfd, buffer + buffer_offset, buffer_left, 0);
		if (rc == 0) {
			syslog(LOG_DEBUG, "%d: Remote host closed socket", sockfd);
			break;
		}
		if (rc == -1) {
			syslog(LOG_DEBUG, "%d: Error on socket", sockfd);
			break;
		}

		buffer_offset += rc;

		char *eol;

		
		
		
		
processline:
		eol = strstr(buffer, "\r\n");
		if (eol == NULL) {
			syslog(LOG_DEBUG, "%d: Haven't found EOL yet", sockfd);
			continue;
		}

		
		eol[0] = '\0';

		if (!inmessage) { 
			printf("C%d: %s\n", sockfd, buffer);

			
			for (i=0; i<4; i++) {
				if (islower(buffer[i])) {
					buffer[i] += 'A' - 'a';
				}
			}
			
			buffer[4] = '\0';

			
			
			
			
			if (STREQU(buffer, "HELO")) { 
				sprintf(bufferout, "250 Ok\r\n");
				printf("S%d: %s", sockfd, bufferout);
				send(sockfd, bufferout, strlen(bufferout), 0);
			} else if (STREQU(buffer, "MAIL")) { 
				sprintf(bufferout, "250 Ok\r\n");
				printf("S%d: %s", sockfd, bufferout);
				send(sockfd, bufferout, strlen(bufferout), 0);
			} else if (STREQU(buffer, "RCPT")) { 
				sprintf(bufferout, "250 Ok recipient\r\n");
				printf("S%d: %s", sockfd, bufferout);
				send(sockfd, bufferout, strlen(bufferout), 0);
			} else if (STREQU(buffer, "DATA")) { 
				sprintf(bufferout, "354 Continue\r\n");
				printf("S%d: %s", sockfd, bufferout);
				send(sockfd, bufferout, strlen(bufferout), 0);
				inmessage = 1;
			} else if (STREQU(buffer, "RSET")) { 
				sprintf(bufferout, "250 Ok reset\r\n");
				printf("S%d: %s", sockfd, bufferout);
				send(sockfd, bufferout, strlen(bufferout), 0);
			} else if (STREQU(buffer, "NOOP")) { 
				sprintf(bufferout, "250 Ok noop\r\n");
				printf("S%d: %s", sockfd, bufferout);
				send(sockfd, bufferout, strlen(bufferout), 0);
			} else if (STREQU(buffer, "QUIT")) { 
				sprintf(bufferout, "221 Ok\r\n");
				printf("S%d: %s", sockfd, bufferout);
				send(sockfd, bufferout, strlen(bufferout), 0);
				break;
			} else { 
				sprintf(bufferout, "502 Command Not Implemented\r\n");
				printf("S%d: %s", sockfd, bufferout);
				send(sockfd, bufferout, strlen(bufferout), 0);
			}
		} else { 
			printf("C%d: %s\n", sockfd, buffer);

			if (STREQU(buffer, ".")) { 
				sprintf(bufferout, "250 Ok\r\n");
				printf("S%d: %s", sockfd, bufferout);
				send(sockfd, bufferout, strlen(bufferout), 0);
				inmessage = 0;
			}
		}

		
		memmove(buffer, eol+2, BUF_SIZE - (eol + 2 - buffer));
		buffer_offset -= (eol - buffer) + 2;

		
		
		if (strstr(buffer, "\r\n")) 
			goto processline;
	}

	
	close(sockfd);
	pthread_exit(NULL);
}


void * get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
