#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

//Headerfil
#include "functions.h"

int miptph(struct clientheader *protocol, int dsock, int size, int psn, int padding)
{
  //struct clientheader *protocol = (struct clientheader*)buffer;

	struct miptp_frame *tp = NULL;
	int headersize = (sizeof(struct miptp_frame)+size);
	tp = malloc(headersize);
	bzero(tp, headersize);
	tp->pl = padding;
	tp->port = protocol->port;
	tp->psn = psn;
	printf("header 14 bit int port: %d\nVanlig port: %d\n", tp->port, protocol->port);
	memmove(tp->file, protocol, size);
	//free(protocol);
	printf("psn er lik: %d\n", tp->psn);
	int ret = writefunction(dsock, (char*)tp, headersize);
	//free(tp);
	printf("local miptph headersize %d\n", headersize);
	return ret;
}

int tell(uint32_t size)
{
	uint32_t usize = size;
	int siz = 0;
	if (usize > 1460)
	{
		siz = usize/1460;
		if (usize%1460)
			siz++;
		printf("antall pakker == %d\n", siz);
	}
	else
		siz++;
	return siz;
}


int getpadding(int size)
{
	if (size < 4)
		return 4-size;
	return size%4;
}




void del(struct que pack[], char *protocol, uint32_t size, int cnt) //size må legges til clientheader
{
	int written = 0;
	int k = 0;
	printf("cnt: %d\n", cnt);
	while(k < cnt)
	{
		if (size >= 1460)
		{
		//pack = malloc(sizeof(struct que));
			pack[k].buf = malloc(1460);
			pack[k].size = 1460;
			pack[k].psn = k;
			pack[k].ack = 0;
			pack[k].pad = 0;
			printf("pack.size %d\n", pack[k].size);
			memmove(pack[k].buf, protocol+written, 1460);
			written += 1460;
			size -= 1460;
		} else if (size < 1460 && size > 0)
		{
			printf("scoobiieeedooo!\n");
		//pack = malloc(sizeof(struct que));
			int padding = getpadding(size);
			pack[k].buf = malloc(size/padding);
			pack[k].size = size/*+padding*/;
			pack[k].psn = k;
			pack[k].ack = 0;
			pack[k].pad = 0;
			printf("pack.size %d\n", pack[k].size);
			memmove(pack[k].buf, protocol+written, size);
			memset(pack[k].buf+size, '\0', padding);
			written += size;
			size -= size;
		}
		k++;
	}
}









int gobackN(char *protocol, int dsock, int size, int port, char dst, char src, int psn, int padding)
{
	struct clientheader *ch = NULL;
	//memset(&ch, 0, size + sizeof(struct clientheader)+1);
	ch = malloc(size + sizeof(struct clientheader));
	//bzero(ch, size + sizeof(struct clientheader));
	ch->size = size;
	ch->port = port;
	ch->dst = dst;
	ch->src = src;
	memmove(ch->buf, protocol, size);
	usleep(1000000);
	int ret = miptph(ch, dsock, ch->size + sizeof(struct clientheader), psn, padding);
	return ret;
}