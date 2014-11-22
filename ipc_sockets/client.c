#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "5000" // the port client will be connecting to 


// get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	// ****************************************************
	// Variables declaration
	// ****************************************************

	int sockfd;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	int infinite_loop = 1;
	char *user_input = malloc(sizeof(char)*BUFSIZ);

	// ****************************************************
	// Setup of sockets communication
	// ****************************************************

	while (infinite_loop) {

		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		// ipconfig getifaddr en1
		if ((rv = getaddrinfo("192.168.0.4", PORT, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		// loop through all the results and connect to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype,
					p->ai_protocol)) == -1) {
				perror("client: socket");
				continue;
			}

			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(sockfd);
				perror("client: connect");
				continue;
			}

			break;
		}

		if (p == NULL) {
			fprintf(stderr, "client: failed to connect\n");
			return 2;
		}

		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
				s, sizeof s);
		printf("client: connecting to %s\n", s);

		freeaddrinfo(servinfo); // all done with this structure

		// ****************************************************
		// Get input and send it to server
		// ****************************************************


		// User is asked for an alpha numeric string input
		printf("Enter an alpha numeric string: ");
		fgets(user_input, BUFSIZ, stdin);

		// If the input string is "quit" the infinite loop is interrupted, so that the socket can be closed
		if (strncmp(user_input, "quit", 4) == 0) {
			infinite_loop = 0;
		}
		
		// Send input to server
		if (send(sockfd, user_input, strlen(user_input), 0) == -1)
			perror("send");

		// Close socket
		close(sockfd);
	}

	return 0;
}
