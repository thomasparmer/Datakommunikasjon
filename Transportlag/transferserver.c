//Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

//Network libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "functions.h" 


int writefile(char* filename, char* file, int size)
{
	FILE *stream = NULL;
	stream = fopen(filename, "a");

	int ret = fwrite(file, sizeof(char), size, stream); //leser inn fra stream

			/*Lukker og frigjør*/

	pclose(stream);
	//free(file);
	return ret;
}


int main(int argc, char *argv[])
{
	if(argc != 5)
	{
		printf("USAGE: %s [socket][filename][size][port]\n", argv[0]);
		return -1;
	}

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
/*	if (bind(usock, (struct sockaddr*)&bindaddr, sizeof(bindaddr)))
	{
		perror("bind");
		return -3;
	}
*/
	if (connect(usock, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) == -1)
	{
		perror("connect");
		return -3;
	}
	int serv = 1;
	int q = 0;
	int siz = strtol(argv[3], (char **)NULL, 10);
	int port = strtol(argv[4], (char **)NULL, 10);
	printf("port %d serv: %d\n", port, serv);
	write(usock, &port, sizeof(int));
	write(usock, &serv, sizeof(int));
	int total = 0;
	for (;;)
	{
		uint32_t size;
		char *buf;
		int ret = read(usock, &size, sizeof(size)); 
		if ( ret < 0) 
		{
			perror("READ");
			return -5;
		} else if (ret == 0)
		{
			close(usock);
			unlink(socketbind);
			free(socketbind);
			exit(0);
		} else
		{
			buf = (char*)malloc(size); //allokerer minne
			printf("size malloc %d\n", size);
			ret = readfunction(usock, buf, size);

			struct clientheader *protocol = (struct clientheader*)buf;
			int returwrite = writefile(argv[2], protocol->buf, ret-sizeof(struct clientheader));
			printf("bytes skrevet til fil: %s,  %d\n", argv[2], returwrite);
			total += returwrite;
			printf("skrevet så langt: %d\n", total);
			if (siz <= total)
			{
				printf("filen er ferdig srevet! \n");
				close(usock);
				unlink(sockname);
				exit(0);
			}
			printf("antall pakker %d\n", q);
			q++;
		}	
	}
	close(usock);
	unlink(sockname);
	return 0;
}