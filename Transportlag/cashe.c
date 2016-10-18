#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"



void addcache(cache **root, char mip, uint8_t mac[6], int socket)
{
	if (!root)
	{
		root = malloc(sizeof(cache));
		(*root)->mipadr = mip;
		(*root)->socket = socket;
		memcpy((*root)->mac, mac, 6*sizeof(uint8_t));
	} else {
		cache *temp = malloc(sizeof(cache));
		temp->mipadr = mip;
		temp->socket = socket;
		memcpy(temp->mac, mac, 6*sizeof(uint8_t));
		temp->next = *root;
		*root = temp;
	}
}


//h
int retmac(cache **root,  char mipadr, 	uint8_t mac[6])
{
	cache *temp = *root;
	while (temp) 
	{	
		if (mipadr == temp->mipadr)
		{
			memcpy(mac, temp->mac, 6*sizeof(uint8_t));
			return temp->socket;
		}
		temp = temp->next;
	}	
	return -1;
}



char retmip(cache **root,  uint8_t mac[6])
{
	cache *temp = *root;
	while (temp) {
		if (mac == temp->mac)
			return temp->mipadr;
		temp = temp->next;
	}
	return '0';
}


/* printing av MAC address*/
void printmac(uint8_t mac[6])
{
	int i;
	for(i = 0; i < 5; ++i)
		printf("%02x:", mac[i]);
	printf("%02x\n", mac[5]);
}

/*printer interfaces*/
void printinterfaces(iface **face) 
{
	iface *tmp = *face;
	int cnt = 0;

	if(tmp == 0) {
		printf("No interfaces stored\n");
	} else {

		while(tmp->next != 0) {
			printf("INTERFACE %d: %s\n", cnt, tmp->name);
			tmp = tmp->next;
			cnt++;
		}
	}
}

void printCache(cache **root)
{
	cache *temp = *root;
	while (temp) 
	{
		printf("mipadresse:%c", temp->mipadr);
		printmac(temp->mac);
		temp = temp->next;
	}
}

