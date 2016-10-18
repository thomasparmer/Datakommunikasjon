
//Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>

//Network libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>

#include "functions.h"


uint32_t cat(char* fil, char** retur) 
{
	char* file = strdup(fil);
	char* path;
	path = malloc(strlen(getenv("PWD"))+strlen(file) + 2);
	printf("sgetenvpath strlen: %zd\n", strlen(getenv("PWD")));
	strcpy(path, getenv("PWD")); 
	strcat(path, "/");
	strcat(path, file);
	free(file);
	printf("path: %s\n", path);
	char* catPath = malloc(strlen("/bin/cat ")+1);
	strcpy(catPath, "/bin/cat ");

	struct stat s;
	stat(path, &s); //path
	uint32_t siz = (s.st_size); //finner lengden på filen jeg skal lese
	printf("siz: %u\n", siz);
	char* cmd = malloc(strlen(catPath) + strlen(path)+1);
	strcpy(cmd, catPath);
	strcat(cmd, path);
	free(path);
	free(catPath);
	FILE *stream = NULL;
	stream = popen(cmd, "r");
	*retur = malloc(siz+1);

	//fprintf(stderr, "retur: %p, *retur: %p\n", retur, *retur);
	printf("retur: %p, *retur: %p\n", retur, *retur);

	int ret = fread(*retur, sizeof(char), siz, stream); //leser inn fra stream
	//fprintf(stderr, "int ret: %d\n", ret);
	printf("int ret: %d\n", ret);
	//*retur[ret] = '\0'; //nullterminerer
			/*Lukker og frigjør*/
	printf("etter nullterminerer\n");

	pclose(stream);
	printf("etter pclose\n");
	free(cmd);
	printf("etter fri cmd\n");
	return siz;
}


int main(int argc, char* argv[]) 
{

	if (argc < 5) 
	{
		printf("Bruk: %s [socket][filnavn][destination adress][port]\n", argv[0]);
		return -1;
	}
	char* retur = NULL;
	printf("retur: %p\n", retur);
	uint32_t size = cat(argv[2], &retur);

	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	fd_set fds, readfds;
	printf("linje78\n");

	const char* sockname = argv[1];
	int usock = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if(usock == -1)
	{
		perror("SOCKET");
		return -2;
	}

	char *socketbind;
	socketbind = malloc(strlen(sockname)+2);
	strcpy(socketbind, "t");
	strcat(socketbind, sockname);
	struct sockaddr_un bindaddr;
	bindaddr.sun_family = AF_UNIX;

	strncpy(bindaddr.sun_path, socketbind, sizeof(bindaddr.sun_path));
	free(socketbind);

	if (connect(usock, (struct sockaddr*)&bindaddr, sizeof(bindaddr)))
	{
		perror("connect");
		return -3;
	}

	int port = strtol(argv[4], (char **)NULL, 10);
	if (port > 16383)
	{
		printf("port number %d too high. max is 16383", port);
		return -4;
	}
	struct clientheader *protocol;
	uint32_t headersize = (sizeof(struct clientheader) + size); //ta bort +1?
	protocol = malloc(headersize);
	bzero(protocol, headersize);
	protocol->dst = argv[3][0];
	protocol->src = argv[1][0];
	protocol->port = port;
	protocol->size = size;
	memcpy(protocol->buf, retur, size);
	free(retur);
	int cli = 0;
	printf("protocol->port %d\n", protocol->port);
	printf("port: %d, cli %d\n", port, cli);
	write(usock, &port, sizeof(port));
	write(usock, &cli, sizeof(cli));
	//write(usock, "pong", 5);


	//uint32_t wrote = write(usock, &headersize, sizeof(wrote));
	//printf("wrote: %u bytes\n", wrote);
	printf("headersize: %u\n", headersize);
	//wrote = write(usock, protocol, headersize);
	int ret = writefunction(usock, (char*)protocol, headersize);
	if (ret < 0) 
	{
		return -5;
	}
	printf("ret: %u bytes\n", ret);



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
			return -6;
		} else if(rc == 0) //Slår inn etter antall sekunder gitt i timeout.tc_sec 
		{
			//lokker socket
			printf("Timeout!\n");
			close(usock);
			return 0;
		}
		//int8_t sizes;
		//char *buf;
	/*	int ret = read(usock, &sizes, sizeof(sizes)); 
		if ( ret < 0) 
		{
			perror("READ");
			return -7;
		}
		buf = (char*)malloc(sizes+1); //allokerer minne
		ret = readfunction(usock, buf, sizes);
	*///	printf("Recieved message: %s\n", buf);
		close(usock);
		return 0;
	}
}
