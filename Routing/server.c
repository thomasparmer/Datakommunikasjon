#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "functions.h" 
int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("USAGE: %s [socket name]\n", argv[0]);
		return -1;
	}


	/**
	*Legger på en "s" for server slik at socketen blir en egen
	*/
	const char* tmp = argv[1];
	char* sockname = malloc(strlen(tmp)+1);
	strcpy(sockname, "s");
	strcat(sockname, tmp);
	int usock = socket(AF_UNIX, SOCK_SEQPACKET, 0);

	if(usock == -1)
	{
		perror("socket");
		return -2;
	}

	struct sockaddr_un bindaddr;
	bindaddr.sun_family = AF_UNIX;
	strncpy(bindaddr.sun_path, sockname, sizeof(bindaddr.sun_path));

	if(bind(usock, (struct sockaddr*)&bindaddr, sizeof(bindaddr)))
	{
		perror("serverbind");
		return -3;
	}

	if(listen(usock, 5))
	{
		perror("listen");
		return -4;
	}

	printf("accept\n");
	int cfd = accept(usock, NULL, NULL);
	for (;;)
	{
		printf("Etter accept\n");
		uint8_t size;
		char *buf;
		int ret = read(cfd, &size, sizeof(size)); 
		if ( ret < 0) 
		{
			perror("READ");
			return -5;
		}
		printf("Etter read size\n");

		buf = (char*)malloc(size+1); //allokerer minne
		ret = readfunction(cfd, buf, size);
		printf("Etter read msg\n");

		printf("Recieved message: %s\n", buf);

		writefunction(cfd, "PONG", 4);
		printf("Etter write\n");
	}

	close(usock);
	unlink(sockname);
}




