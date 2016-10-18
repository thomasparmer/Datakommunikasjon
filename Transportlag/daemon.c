/**
 *inkluderer standard biblioteker
 *#1 printf
 *#2 read & write
 *#3 malloc
 *#4 strcpy
 *#5 standariserte int lengder
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/**
 *inkluderer nettverks biblioteker
 *#1 socket
 *#2 socket
 *#3 select
 *#4 sockaddr_un
 *#5 sockaddr_ll
 *#6 ETH_P_ALL
 *#7 htons
 *#8 if_nametoindex
 *#9 timeout select
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <net/if.h>
 #include <time.h>



/**
 *inkluderer header fil(er)
 *functions.c
 *send&recv.c
 */
#include "functions.h"
#define ETH_P_MIP 65535 

/**
 *main MIPdaemon
 *Starter en select basert  server
 */
 int main(int argc, char const *argv[])
 {
 	if (argc != 2)
 	{
 		printf("USAGE: %s [MIPHOSTADR]\n", argv[0]);
 		return -1;
 	}

//Deklarerer datastrukturer og adresser
 	const char* sockname = argv[1];
 	int i, rc;
 	int usock, ssock;
 	struct timeval timeout;
 	timeout.tv_sec = 10;
 	timeout.tv_usec = 0;

 	fd_set fds, readfds, /*writefds, */rfds;

 	iface *inter;
 	iface *tmp;
 	uint32_t size = 0;
 	ssize_t recvd = 0;
 	char* buf;
 	char packet[1500];
 	static int total = 0; 	
 	int sd = 0;
 	int lines = 0;
 	int dlagret = 0;
 	int value;
 	uint8_t dstm[6];
 	struct miptp_frame  *lager;

	/**
	 *Oppretter socket til server
	 */
	 ssock = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	 if (ssock == -1)
	 {
	 	perror("socket");
	 	return -2;
	 }


	/**
	 *binder socket 
	 *parameter+s
	 */
	 char *sockserv;
	 sockserv = malloc(strlen(sockname)+2);
	 strcpy(sockserv, "s");
	 strcat(sockserv, sockname);
	 struct sockaddr_un baddr;
	 baddr.sun_family = AF_UNIX;
	 strncpy(baddr.sun_path, sockserv, sizeof(baddr.sun_path));





	 struct miparpcache *root = NULL;
	 root = malloc(sizeof(cache));

	/**
	 *Oppretter en unix domain socket
	 *localhost
	 */
	 usock = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	 if (usock == -1)
	 {
	 	perror("socket");
	 	return -2;
	 }



	/**
	 *binder socket 
	 *interface gitt som parameter
	 */
	 struct sockaddr_un bindaddr;
	 bindaddr.sun_family = AF_UNIX;
	 strncpy(bindaddr.sun_path, sockname, sizeof(bindaddr.sun_path));

	 if (bind(usock, (struct sockaddr*)&bindaddr, sizeof(bindaddr)))
	 {
	 	perror("clientbind");
	 	return -3;
	 }

	/**
	 *Lytter på socket
	 */
	 if(listen(usock, SOMAXCONN))
	 {
	 	perror("listen");
	 	return -4;
	 }




	/*Finner interface*/
	 inter = malloc(sizeof(struct interfaces));
	 int teller = get_interface(&inter);

  	/*Ser om vi fant interfaceet*/
	 if(teller == 0) {
	 	perror("get_interface");
	 	return -6;
	 }
	 lines = teller;

	 uint8_t hmac[lines][6];

	#ifdef DEBUG
	 printinterfaces(&inter);
	#endif

	 tmp = inter;
	 for (i = 0; i < teller; i++)
	 {
	 	int ret = get_if_hwaddr(usock, tmp->name, hmac[i]); /*&hmac*/
	 	//addcache(&root, sockname[0], hmac[i]);
	 	tmp = tmp->next;
  		/*Ser om vi fant host mac adress */
	 	if(ret == -1) {
	 		perror("get_if_hwaddr");
	 		return -7;
	 	}
	 }

	// uint8_t test[6];
	// retmac(&root, sockname[0], test);
 	 /*Prints out the local MAC address*/
	#ifdef DEBUG
	 for (i = 0; i < teller; i++)
	 {
	 	printf("Local MAC: ");
	 	printmac(hmac[i]);
	 }
	#endif

	/**
	*Setter opp raw socket
	*Binder socket til interface
	*/
	int rawsock[lines];
	struct sockaddr_ll device[lines];
	tmp = inter;
	for (i = 0; i < lines; i++)
	{
		if ((rawsock[i] = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_MIP))) == -1) //htons ikke nødvendig (11111)
		{
			perror("Rawsocket");
			return -5;
		}

		memset(&device[i], 0, sizeof(device[i]));

		device[i].sll_family = AF_PACKET;
		device[i].sll_ifindex = if_nametoindex(tmp->name);
		tmp = tmp->next;

		if (bind(rawsock[i], (struct sockaddr*)&device[i], sizeof(device[i])))
		{
			perror("bind");
			while(i-- >= 0)
				close(rawsock[i]);
			return -6;
		}
	}



	/**
	*Initialiserer fd set
	*/
	FD_ZERO(&fds);
	FD_ZERO(&rfds);

	FD_SET(usock, &fds);
	for (i = 0; i < lines; i++)
		FD_SET(rawsock[i], &fds);
	int highest = usock;

	for (i = 0; i < lines; i++)
	{
		if (highest < rawsock[i])
			highest = rawsock[i];
	}
	int hide = 1;
	int hidec = 1;
	for (;;)
	{
		if (hide)
		{
			if (connect(ssock, (struct sockaddr*)&baddr, sizeof(baddr)) == 0)
			{
				hide = 0;
				FD_SET(ssock, &fds);
				printf("connected to server\n");
				char *te = malloc(5);
				strcpy(te, "test");
				writefunction(ssock, te, 5);
			}
			//else
			  //perror("connect");
		}
		if (sd > 0)
		{
			FD_SET(sd, &fds);
			if(sd > highest)
				highest = sd;
		}
		readfds = fds;

		rc = select(highest+1, &readfds, NULL/*&writefds*/, NULL, &timeout); 
		if (rc<0)
		{
			perror("select");
			return -5;
		}
		if (rc == 0)
		{
			timeout.tv_sec = 5;
			continue;
		}
		printf("for isset usock\n");
		if (FD_ISSET(usock, &readfds))
		{
			printf("isset uscok\n");
			if (hidec) 
			{
				hidec = 0;
				printf("accept\n");
				sd = accept(usock, NULL, NULL);
				if (sd == -1)
				{
					perror("accept");
					return -6;
				}
				printf("sd accept: %d\n", sd);
				//writefunction(sd, "Daemon", 6);
			}
		}

		if (FD_ISSET(sd, &readfds))
		{
			printf("incomming message on socket %d\n", sd);
			recvd = read(sd, &size, sizeof(size));
			if (recvd < 0)
			{
				perror("read");
				return -8;
			} else if (recvd == 0) 
			{
				close(sd);
				sd = 0;
				hidec = 1;
				FD_CLR(sd, &fds);
				FD_CLR(sd, &readfds);

			} else {
				buf = malloc(size);
				recvd = readfunction(sd, buf, size);

			/*	#ifdef DEBUG
				printf("Fra unix socket: %s\n", buf);
				#endif
*/
				lager = (struct miptp_frame*)buf;

				dlagret = recvd;
				//lagret = malloc(dlagret);
				//memcpy(lagret, lager, recvd);

				//lagret = strdup(buf);
				//strcpy(lagret, buf);
				//printf("lagret %s", lagret);
				printf("motatt dlagret %d\n", dlagret);
				struct miptp_frame  *tp = (struct miptp_frame*)buf;
				struct clientheader *ch = (struct clientheader*)tp->file;

				value = retmac(&root, ch->dst, dstm);

				if(value != -1) 
 					{	    //Sender om vi fant mac adressen
 						arp(ch->src, ch->dst, rawsock[value], hmac[value], dstm, tp,  4, recvd);
					 /**
					 *Sender til mip-adressens tilhørende macadresse
					 */

					} else {
					/**
 					*Sender ARP-Broadcast hvis mip adressen ikke ligger i Cache
 					*/
 					for (i = 0; i < lines; i++)
 						arp(ch->src, ch->dst, rawsock[i], hmac[i], NULL, NULL, 1, 0);
 				}
 			}
 		}

 		for (i = 0; i < lines; i++)
 		{
 			if (FD_ISSET(rawsock[i], &readfds))
 			{
 				recvd = recv(rawsock[i], packet, sizeof(packet), 0);
 				struct ether_frame *frame = (struct ether_frame*)packet;
 				struct mip* protocol =  (struct mip*)frame->contents;

				#ifdef DEBUG
 				printf("Tok imot %zu bytes fra rawsock: %d\n", recvd, rawsock[i]);
				#endif

				/** 
				*Skjekker TRA
				*/

				/*Mottar broadcast*/
				if (protocol->tra == 1)
				{
					#ifdef DEBUG
					printf("ARP Broadcast received from: %c\n", protocol->src);
					printf("ARP Broadcast ment for: %c\n", protocol->dst);
					#endif

					 /*Er broadcasten ment for denne daemon*/
					if(protocol->dst == sockname[0]) 
					{
					   /*Ser om senderen ligger i MIP-ARP Cache*/
						uint8_t sendermac[6];
						if(retmac(&root, protocol->src, sendermac) == 0)
							addcache(&root, protocol->src, frame->src_addr, i);

	  					   /* ARP response */
						printf("ARP-RESPONSE\n");
						arp(protocol->dst, protocol->src, rawsock[i], hmac[i], frame->src_addr, NULL, 0, 0);
					} else
					printf("Melding ikke ment for denne mip adressen protocol->dst: %c\n", protocol->dst);

				} else if(protocol->tra == 2)
				{
					printf("routing funksjonalitet tatt bort\n");
				} else if(protocol->tra == 4)
				{
					printf("tra ======= 4\n");
					if(protocol->dst == argv[1][0]) 
					{
						#ifdef DEBUG
						printf("Sending message to server\n");
						#endif
	 					/**
	 					*Sender til server over unix domain socket, og mottar pong
	 					*/
	 					total += (recvd-sizeof(struct mip)-sizeof(struct ether_frame));	 					
	 					printf("ferdig write, size totalt: %d\n", total);
	 					int skrevet = writefunction(sd, protocol->buf, recvd-sizeof(struct mip)-sizeof(struct ether_frame));
	 					printf("skrevet %d\n", skrevet);
	 				} else
	 				{
	 					printf("pakken ikke ment for denne daemon\n");
	 				}

	 			} else if (protocol->tra == 0)
	 			{
					#ifdef DEBUG
	 				printf("ARP RESPONSE motatt fra: %c\n", frame->contents[2]);
	 				printf("ARP RESPONSE SEndt to: %c\n", protocol->dst);
					#endif
					/*er meldingen ment for denne daemon*/
	 				if(frame->contents[3] == sockname[0]) {
						/*Legger inn i MIP-ARP Cache*/
	 					uint8_t check[6];
	 					if (retmac(&root, frame->contents[2], check) == -1)
	 						addcache(&root, frame->contents[2], frame->src_addr, i);
	 				 	 /*Sender melding til mac*/

	 					printf("storrelse: %d\n", dlagret);



	 					arp(frame->contents[3], frame->contents[2], rawsock[i], hmac[i], frame->src_addr, /*(char*)*/lager,  4, dlagret);

	 				} else
	 				{
	 					printf(" arp response ikke for denne daemon\n");
	 				}
	 			}
	 		}
	 	}
	 }
	 close(usock);
	//close(rawsock);
	 unlink(sockname);
	 return 0;
	}


