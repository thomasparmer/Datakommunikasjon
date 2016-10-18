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
 	int usock, ssock, numsocks;
 	struct timeval timeout;
 	timeout.tv_sec = 10;
 	timeout.tv_usec = 0;

 	fd_set fds, readfds, /*writefds, */rfds;

 	iface *inter;
 	iface *tmp;
 	int8_t size = 0;
 	ssize_t recvd = 0;
 	char* buf;
 	char packet[1500];
 	int sd[2];
 	numsocks = 0;
 	int lines = 0;
 	int indx = 0;
 	char* lagret;
 	struct mip *lager;
 	int value;
 	uint8_t dstm[6];

 	struct routing rt[4];
 	struct routing rt_horizon[4];
 	for(i = 0; i < 4; i++) 
 	{
 		memset(&rt[i], 0, sizeof(rt[i]));
 		rt[i].host = argv[1][0];
 		memset(&rt_horizon[i], 0, sizeof(rt_horizon[i]));
 	}


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
	 sockserv = malloc(strlen(sockname)+1);
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

  /*Constructing the timeout table for handling timeouts with connections*/
	 struct link_timeout l_table[lines];

  /*Constructing raw socket table to handle the mip and raw socket connection*/
	 char rtb[lines];

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
	int client = 0;
	int highest = usock;

	for (i = 0; i < lines; i++)
	{
		if (highest < rawsock[i])
			highest = rawsock[i];
	}
	int hide = 1;
	for (;;)
	{
		//printf("for(;;)\n");
		if (hide)
		{
			if (connect(ssock, (struct sockaddr*)&baddr, sizeof(baddr)) == 0)
			{
				hide = 0;
				FD_SET(ssock, &rfds);
				printf("connected to server\n");
				writefunction(ssock, "test", 4);
				char* servbuf = malloc(5);
				readfunction(ssock, servbuf, 5);

				printf("%s\n", servbuf);
			}
			else
				perror("connect");
		}

		readfds = fds;

    	/*Checks if there is a timeout on any of the connections and removes the node if any*/
		time_t tidtimeout;
		time_t now = time(NULL);
		for(i = 0; i < lines; i++)  {

			tidtimeout = now - l_table[i].time;

			if(tidtimeout > 20 && l_table[i].mip != 0) {
				bortfallendenode(l_table[i].mip, rt, l_table, lines);   	
			}      
		}

    /*prints out the routing table*/
#ifdef DEBUG
		printroutingtabell(rt[0].host, rt);
#endif


		rc = select(highest+1, &readfds, NULL/*&writefds*/, NULL, &timeout); 
		if (rc<0)
		{
			perror("select");
			return -5;
		}
		if (rc == 0)
		{
			for (i = 0; i < lines; i++)
			{
				if (rtb[i] == 0) 
					arp(rt, argv[1][0], '\0', rawsock[i], hmac[i], NULL, NULL, 2, 0);
				else
				{
					split_horizon(rt, rt_horizon, rtb[i]);
					arp(rt_horizon, argv[1][0], '\0', rawsock[i], hmac[i], NULL, NULL, 2, 0);
				}
			}
			timeout.tv_sec = 20;
			continue;
		}
		if (FD_ISSET(usock, &readfds))
		{

			sd[client] = accept(usock, NULL, NULL);
			if (sd[client] == -1)
			{
				perror("accept");
				return -6;
			}
			recvd = read(sd[client], &size, sizeof(size));
			if (recvd < 0)
			{
				perror("read");
				return -8;
			} else if (recvd == 0) 
			{
				close(sd[client]);
				FD_CLR(sd[client], &fds);
			} else {
				buf = malloc(size+1);
				recvd = readfunction(sd[numsocks], buf, size);
				recvd = writefunction(sd[numsocks], "DAEMON", 6);
					#ifdef DEBUG
				printf("Fra unix socket: %s\n", buf);
					#endif
				lagret = malloc(strlen(buf)+1);
				strcpy(lagret, buf);

				value = retmac(&root, buf[3], dstm);

 					if(value != -1) 
 					{	    //Sender om vi fant mac adressen
 						arp(NULL, argv[1][0], buf[2], rawsock[value], hmac[value], dstm, buf,  4, 1);
					 /**
					 *Sender til mip-adressens tilhørende macadresse
					 */
					}else if(nexthop(buf[2], rt) != -1)
					{
						indx = nexthop(buf[2], rt);
						/*Ser om mac til neste hopp er kjent og sender msg */
						value = retmac(&root, rt[indx].rout, dstm);
						if(value != -1) {
							arp(NULL, argv[1][0], buf[2], rawsock[value], hmac[value], dstm, buf, 4, rt[indx].length);

	 					   /*sends ARP-Broadcast to get the next hop MAC address*/
						} else {
							for (i = 0; i < lines; i++)
								arp(NULL, argv[1][0], rt[indx].rout, rawsock[i], hmac[i], NULL, NULL, 1, 0);
						}


					}

					else {
						/**
 					*Sender ARP-Broadcast hvis mip adressen ikke ligger i Cache
 					*/
						/*Sender broadcast hvis mip adressen ikke finnes i cache*/
 					for (i = 0; i < lines; i++)
 						arp(NULL, argv[1][0], buf[2], rawsock[i], hmac[i], NULL, NULL, 1, 0);

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
						arp(NULL, protocol->dst, protocol->src, rawsock[i], hmac[i], frame->src_addr, NULL, 0, protocol->ttl);
					} else
					printf("Melding ikke ment for denne mip adressen protocol->dst: %c\n", protocol->dst);

				} else if(protocol->tra == 2)
				{


					if (recvd > 0) 
					{
						#ifdef DEBUG
						printf("motatt routing fra klient %c\n", protocol->src);
						printf("motatt %zd bytes", sizeof(protocol->buf));
						#endif
						int j;


						update_time(l_table, protocol->src, lines);
						rtb[i] = protocol->src;
						struct routing *tmp = (struct routing*)protocol->buf;



						 /* Ser først etter om naboen er lagret */
						if(rt[0].dest != protocol->src && rt[1].dest != protocol->src && rt[2].dest != protocol->src && rt[3].dest != protocol->src) {
							for(i = 0; i < 4; i++) {	    
								if(rt[i].dest == 0) {
									rt[i].dest = protocol->src;
									rt[i].length = 1;
									rt[i].rout = protocol->src;
									break;
								}
							}
						}

					 /*Ser om det er noen nye forbindelser */
						for(i = 0; i < 4; i++) {
							if(rt[0].dest != tmp[i].dest && rt[1].dest != tmp[i].dest && rt[2].dest != tmp[i].dest && rt[3].dest != tmp[i].dest && rt[0].host != tmp[i].dest) {	  
								for(j = 0; j < 4; j++) {
									if(rt[j].dest == 0) {
										rt[j].dest = tmp[i].dest;
										rt[j].rout = protocol->src;
										rt[j].length = tmp[i].length+1;
										break;
									}
								}   
							}	  
						}

						 /*checks if there is any shorter connections*/
						for(i = 0; i < 4; i++) {
							for(j = 0; j < 4; j++) {
								if(rt[j].dest == tmp[i].dest && (rt[j].length - tmp[i].length) > 1) {
									rt[j].rout = protocol->src;
									rt[j].length = tmp[i].length+1;
								}
							}
						}

						 /*checks if the neighbour is stored as a rout from another mip*/
						for(i = 0; i < 4; i++) {
							if(rt[i].dest == protocol->src && rt[i].length > 1) {
								rt[i].length = 1;
								rt[i].rout = protocol->src;
							}
						}	 
					}

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
	 					if (!hide) 
	 					{
	 						writefunction(ssock, protocol->buf, protocol->payload);

	 						char *retu;
	 						printf("før read\n");
	 						
	 						retu = malloc(5);
	 						readfunction(ssock, retu, 5);
	 					}
	 				} else
	 				{
	 					 /*ser etter i routing tabell*/
	 					indx = nexthop(protocol->dst, rt);

	 					if(indx == -1) {
	 						printf("Ingen forbindelse med destinasjon, kaster pakken\n");

	 					} else if(protocol->ttl < 1) {
	 						printf("Time to live overgått, kaster pakken\n");
	 					} else {

	 						value = retmac(&root, rt[indx].rout, dstm);

	    					 /*Sender meldingen om MAC addressen er kjent*/
	 						if(value != -1) {
	 							arp(NULL, protocol->src, protocol->dst, rawsock[value], hmac[value], dstm, protocol->buf, 4, protocol->ttl);

	       					/*Sender ARP broadcast for å finne mac addressen*/	       
	 						} else {

	       						/*Stores the mip frame for later use*/
	      						 //lager = (struct mip_frame*)frame->contents;
	 							lager = malloc(sizeof(struct mip)+strlen(protocol->buf));
	 							lager->tra = protocol->tra;
	 							lager->ttl = protocol->ttl;
	 							lager->payload = protocol->payload;
	 							lager->src = protocol->src;
	 							lager->dst = protocol->dst;
	 							memcpy(lager->buf, protocol->buf, strlen(protocol->buf));

	 							for(i = 0; i < lines; i++)
	 								arp(NULL, argv[1][0], rt[indx].rout, rawsock[i], hmac[i], NULL, NULL, 1, 0); 
	 						}
	 					}
	 				}

	 			} else if (protocol->tra == 0)
	 			{
					#ifdef DEBUG
	 				printf("ARP RESPONSE motatt fra: %c\n", frame->contents[2]);
					#endif
					/*er meldingen ment for denne daemon*/
	 				if(frame->contents[3] == sockname[0]) {
						/*Legger inn i MIP-ARP Cache*/
	 					uint8_t check[6];
	 					if (retmac(&root, frame->contents[2], check) == -1)
	 						addcache(&root, frame->contents[2], frame->src_addr, i);
	 				 	 /*Sender melding til mac*/
	 					arp(NULL, frame->contents[3], frame->contents[2], rawsock[i], hmac[i], frame->src_addr, lagret,  4, protocol->ttl);

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


