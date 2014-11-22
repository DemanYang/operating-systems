#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

#define PORT "5000"  // the port users will be connecting to
#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once 

// function for finding the number of digits in a string
int digits_in_str(char *s)
{
    int i = 0;
    while (*s != '\0') {
    	if (isdigit(*s)) // counting number of digits
       		i++;
    s++;
    }

    return i;
}

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
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

	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	int numbytes;
	char buf[MAXDATASIZE];
	int infinite_loop = 1;
	FILE *digits;

	// ****************************************************
	// Output file 'digits.out' created
	// ****************************************************

	digits = fopen("digits.out","w");
	fclose(digits);

	// ****************************************************
	// Setup of sockets communication
	// ****************************************************

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	// ****************************************************
	// User enters input in an infinite loop
	// ****************************************************

	while(infinite_loop) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		// ****************************************************
		// Get input string from client, count number of digits
		// and output them in the file 'digits.out'
		// ****************************************************

		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}

		buf[numbytes] = '\0';

		printf("SERVER: received '%s'\n",buf);


		// If the input string is "quit" the infinite loop is interrupted
		if (strncmp(buf, "quit", 4) == 0) {
			infinite_loop = 0;
		}

		// The 'digits.out' file is opened so that the buffer can be appended into it
		digits = fopen ("digits.out","a");

		// The number of digits in the input string is written in the output 'digits.out' file
		fprintf(digits, "%d: ", digits_in_str(buf));
		// Then, the input string is written in the output 'digits.out' file
		fputs(buf, digits);
		// The output file is closed
		fclose(digits);
		// Close socket that was accepted, so that it can accept the next one
		close(new_fd);
	}

	return 0;
}
