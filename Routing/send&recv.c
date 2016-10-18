#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

/*
Metode for å lese 
har først lest en størrelse type int8_t so bestemmer hvor stor meldingen er.
leser i en løkke helt til det som er lest er like stort som størrelsen som ble sendt

*/
int readfunction(int socket, char *buf, int8_t size) 
{
	int cnt = 0;
	while (cnt < size)
	{
		int ret = read(socket, buf + cnt, size -  cnt);
		if (ret < 0) 
		{
			perror("READ");
			return -5;
		}
		cnt += ret;
	}
	return cnt;
}

/*
sendefunskjon
sender først størrelsen på bøfferet som en int8_t
sender så bøfferet i en løkke helt til variabelen som 
teller det som er sendt er like stor som størrelsen
på bøfferet
*/

int writefunction(int socket, char *buffer, int8_t siz) 
{

	int8_t size = siz;
	int cnt = 0;
	printf("sending size %d\n", size);
	int ret = write(socket, &size, sizeof(size));
	if (ret < 0) {
		perror("write");
		return -3;
	}


	while (cnt < size) 
	{
		printf("sending %s\n", buffer);
		ret = write(socket, buffer + cnt, size - cnt);
		if (ret < 0) 
		{
			perror("WRITE");
			return -4;
		}
		cnt += ret;
	}
	printf("%s\n", "FULLFØRER WRITE!\n");
	return cnt;
}