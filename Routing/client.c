//Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>

//Network libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>

#include "functions.h"

int main(int argc, char* argv[]) 
{

	if (argc < 4) 
	{
		printf("USAGE: %s [HOST ADRESS][RECV ADRESS][MESSAGE]\n", argv[0]);
		return -1;
	}

	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	fd_set fds, readfds;

	const char* sockname = argv[1];
	int usock = socket(AF_UNIX, SOCK_SEQPACKET, 0);

	if(usock == -1)
	{
		perror("SOCKET");
		return -2;
	}

	struct sockaddr_un bindaddr;
	bindaddr.sun_family = AF_UNIX;

	strncpy(bindaddr.sun_path, sockname, sizeof(bindaddr.sun_path));

	if (connect(usock, (struct sockaddr*)&bindaddr, sizeof(bindaddr)))
	{
		perror("connect");
		return -3;
	}

	char *msg;
	int i;
	int alloc = 0;

	for (i = 1; i < argc; i++)
		alloc += strlen(argv[i]);

	msg = malloc(alloc+argc+1);

	for (i = 1; i < argc; i++)
	{
		strcat(msg, argv[i]);
		strcat(msg, " ");
	}

	//char *mip;
	//mip = transport(argv[1][0], argv[2][0], alloc);

	int ret;

	//char *send;
	//send = malloc(strlen(mip)+strlen(msg)+1);
	//strcat(send, mip);
	//strcat(send, msg);
//	free(mip);
//	free(msg);
	printf("send: %s\n", msg);
	ret = writefunction(usock, msg, strlen(msg));
	if (ret < 0) 
	{
		return ret;
	}

	/**
	*Initialiserer fd set
	*/
	FD_ZERO(&fds);
	FD_SET(usock, &fds);

	for (;;)
	{
		readfds = fds;
		int rc = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout); //&timeout 
		if (rc<0)
		{
			perror("select");
			return -5;
		} else if(rc == 0) //Slår inn etter antall sekunder gitt i timeout.tc_sec 
		{
			//lokker socket
			printf("Timeout!\n");
			close(usock);
			return 0;
		}

		int8_t size;
		char *buf;
		ret = read(usock, &size, sizeof(size)); 
		if ( ret < 0) 
		{
			perror("READ");
			return -5;
		}

		buf = (char*)malloc(size+1); //allokerer minne
		ret = readfunction(usock, buf, size);

		printf("Recieved message: %s\n", buf);


		close(usock);
		return 0;
	}
}
