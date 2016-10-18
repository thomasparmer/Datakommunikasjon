#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "functions.h"



/*

void addque(struct que **root, struct clientheader *ch, int psn)
{
	uint32_t size = sizeof(struct clientheader) + ch->size + sizeof(struct que);
	printf("storrelse: %d\n", size);
	if((*root) == 0)
	{
		printf("\n\n\n\nSCORRRE!!!!!!!!!!\n\n\n\n\n\n");
		(*root) = malloc(size);
		bzero((*root), size);
		printf("size %d\n", size);
		(*root)->size = size - sizeof(struct que);
		(*root)->psn = psn;
		(*root)->ack = 0;
		(*root)->next = malloc(sizeof(struct que));
		(*root)->next = 0;
		memmove((*root)->buf, ch, size);
		printf("%s\n", (*root)->buf);
		struct clientheader *chs = (struct clientheader *)(*root)->buf;
		printf("%d\n", chs->port);
	} else
	{
		struct que *temp2 = malloc(size);
		bzero(temp2, size);
		temp2->size = size - sizeof(struct que);
		temp2->psn = psn;
		temp2->ack = 0;
		temp2->next = malloc(sizeof(struct que));
		temp2->next = 0;
		memmove(temp2->buf, ch, size);
		temp2->next = (*root);
		(*root) = temp2;
		printf("%s\n", (*root)->buf);
		struct clientheader *chs = (struct clientheader *)(*root)->buf;
		printf("%d\n", chs->port);
	}
}
*/
/*
void ack(struct que **root, int psn)
{
	struct que *temp = *root;
	while (temp->psn != psn)
		temp = temp->next;
	temp->ack = 1;
}

*/
/*

struct clientheader* retpsn(struct que **root, int psn)
{
	struct que *temp = *root;
	if (!temp)
		return NULL;
	else
	{
		struct clientheader *ch = NULL;//(struct clientheader*)array;
		//char array[1500];
		while (temp->psn != psn)
		{
			if (!temp)
				return NULL;
			temp = temp->next;
			printf("temp psn %d temp size %d", temp->psn, temp->size);
			if (!temp)
				return NULL;
		}

		ch = malloc(temp->size);
		bzero(ch, temp->size);
		memmove(ch, (struct clientheader*)temp->buf, temp->size);
		ch = (struct clientheader*)temp->buf;
		printf("port i retnext %d size i retnext %d\n", ch->port, ch->size);
		return ch;
	}
}

struct clientheader* retnext(struct que **root)
{
	struct que * temp = *root;
	struct que * prev = NULL;

	if (!temp)
		return NULL;
	else
	{
		struct clientheader *ch = NULL;//(struct clientheader*)array;
		
		while (temp->ack != 1)
		{
			prev = temp:
			printf("temp psn %d temp size %d", temp->psn, temp->size);
			temp = temp->next;
			if (!prev)
				return NULL;
		}

		ch = malloc(prev->size);
		bzero(ch, prev->size);
		memmove(ch, (struct clientheader*)prev->buf, prev->size);
		//ch = (struct clientheader*)prev->buf;
		printf("port i retnext %d size i retnext %d\n", ch->port, ch->size);
		return ch;
	}
}
*/