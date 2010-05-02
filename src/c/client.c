#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "protocol.h"

/* port we're listening on */
#define PORT 7674

int main(int argc, char *argv[]) {
	/* server address */
	struct sockaddr_in serveraddr;
	struct hostent *he;
	
	/* client address */
	struct sockaddr_in clientaddr;
	
	/* socket descriptor for connector */
	int connector;
	
	char buf[10240];
	int nbytes;
	
	protocol prot;
	
	// get the host info
	if((he=gethostbyname(argv[1])) == NULL) {
		perror("gethostbyname()");
		exit(1);
	}
	printf("Client-gethostbyname is OK...\n");
	
	/* get the listener */
	if((connector = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Client-socket() error lol!");
		/*just exit lol!*/
		exit(1);
	}
	printf("Client-socket() is OK...\n");
	
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	memset(&(serveraddr.sin_zero), '\0', 8);
	
	if(connect(connector, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr)) == -1) {
		perror("Client-connect() error");
		exit(1);
	}
	printf("Client-connect() is OK...\n");
	
	while(1) {
		if((nbytes = recv(connector, (void *)& prot, 10000, 0)) == -1) {
			perror("Client-recv()");
			exit(1);
		}
		printf("protocol data size=%d\n", nbytes);
		printf("name=%s\n", prot.name);
		printf("version=%d\n", prot.version);
		printf("pkt_len=%d\n", prot.pkt_len);
		printf("service=%d\n", prot.service);
		printf("status=%d\n", prot.status);
		memcpy(buf, prot.data, prot.pkt_len-20);
		printf("data=%s\n", prot.data);
		memset(prot.data, '\0', sizeof(prot.data));
	}
	
	close(connector);
	
	return 0;
}
