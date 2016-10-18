#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <inttypes.h>
#include <sys/un.h>
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros


//Headerfil
#include "functions.h"



int main(int argc , char *argv[])
{
	
	if (argc < 3)
	{
		printf("USAGE: %s [socket][timeout]\n", argv[0]);
		return -1;
	}

	int usock, dsock, new_socket, client_socket[10][4], max_clients = 10, rc, i, sd, hide = 1;
	int window = 10;
	int highest;
	uint32_t valread;
	uint32_t siz;
	const char* sockname = argv[1];

	char *buffer;//[1492];


	int x = 0;
	struct lager hoved[max_clients];
	while (x < max_clients)
	{
		bzero(&hoved[x], sizeof(struct lager));
		hoved[x].ack = -1;
		x++;
	}

	struct que *root[max_clients];
	for (i = 0; i < max_clients; i++)
	{
		memset(&root[i], 0, sizeof(struct que));
		root[i] = malloc(sizeof(struct que));
		root[i] = 0;
	}
	


    //set av socket descriptorer
	fd_set readfds, fds;

    //timeout struct til select
	int sectime = strtol(argv[2], (char **)NULL, 10);
	struct timeval timeout;
	timeout.tv_sec = sectime;
	timeout.tv_usec = 0;

	memset(client_socket, 0, sizeof(client_socket));

//Oppretter socket
	usock = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if(usock == -1)
	{
		perror("socket");
		return -2;
	}

	/**
     *Type socket spesifisert	
	 *binder socket 
	 *parameter+t for connections from transportclient
	 */
	 char *sockserv;
	 sockserv = malloc(strlen(sockname)+2);
	 strcpy(sockserv, "t");
	 strcat(sockserv, sockname);
	 struct sockaddr_un bindaddr;
	 bindaddr.sun_family = AF_UNIX;
	 strncpy(bindaddr.sun_path, sockserv, sizeof(bindaddr.sun_path));
	 free(sockserv);
	 if(bind(usock, (struct sockaddr*)&bindaddr, sizeof(bindaddr)))
	 {
	 	perror("serverbind");
	 	return -3;
	 }

    //Setter maksimum 5 påventende tilkoblinger til usock
	 if (listen(usock, 5) < 0)
	 {
	 	perror("listen");
	 	return -4;
	 }

	/**
	 *Oppretter socket til daemontilkobling
	 */
	 dsock = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	 if (dsock == -1)
	 {
	 	perror("socket");
	 	return -2;
	 }
	 /**
	 *binder socket 
	 *parameter
	 */
	 struct sockaddr_un baddr;
	 baddr.sun_family = AF_UNIX;
	 strncpy(baddr.sun_path, sockname, sizeof(baddr.sun_path));


    //Setter fildeskriptor settet til 0
	 FD_ZERO(&fds);
	 FD_ZERO(&readfds);

    //Legger inn u sock i fildeskriptor settet
	 FD_SET(usock, &fds);
	 highest = usock;

	 for (;;)
	 {


	 	if (hide)
	 	{
	 		if (connect(dsock, (struct sockaddr*)&baddr, sizeof(baddr)) == 0)
	 		{
	 			hide = 0;
	 			FD_SET(dsock, &fds);
	 			if(dsock > highest)
	 				highest = dsock;
	 			printf("connected to Daemon\n");
	 		}
	 	}


	 	readfds = fds;
        //Legger til child sockets til fildeskriptor settet
	 	for ( i = 0 ; i < max_clients; i++)
	 	{
            //socket descriptor
	 		sd = client_socket[i][0];

            //Legger til i settet om deskriptor eksisterer
	 		if(sd > 0)
	 			FD_SET(sd, &readfds);

            //Lagrer høyeste fil deskriptor verdi til bruk i select setningen
	 		if(sd > highest)
	 			highest = sd;
	 	}
        //Blokker til aktivitet på en av socketene
	 	rc = select(highest+1, &readfds, NULL, NULL, &timeout);
	 	if (rc < 0)
	 	{
	 		printf("select error");
	 		return -5;
	 	}
	 	if (rc == 0)
	 	{
	 		timeout.tv_sec = sectime;
	 		int j = 0;
	 		i = 0;
	 		//for (i = 0; i < max_clients; i++)
	 		while (i < max_clients)
	 		{
	 			if (hoved[i].ack != -1)
	 			{
	 				int count;
	 				int pac = hoved[i].cnt - hoved[i].ack;

	 				if (pac > window)
	 					pac = 10;

	 				//for (j = 0; j < pac; j++)
	 				while (j < pac)
	 				{
	 					count = gobackN(hoved[i].qued[hoved[i].ack+j].buf, dsock, hoved[i].qued[hoved[i].ack+j].size, hoved[i].port, hoved[i].dst, hoved[i].src, hoved[i].qued[hoved[i].ack+j].psn, hoved[i].qued[hoved[i].ack].pad);
	 					j++;
	 				}
	 			}
	 			i++;
	 		}
	 		i = 0;
	 		continue;
	 	}

        //Om noe skjer på usock, betyr det en ny client tilkobling
	 	if (FD_ISSET(usock, &readfds))
	 	{
	 		printf("ny connection\n");
	 		new_socket = accept(usock, NULL, NULL);
	 		if (new_socket == -1)
	 		{
	 			perror("accept");
	 			return -6;
	 		} 

	 		for (i = 0; i < max_clients; i++)//Legger til ny socket i array av sockets
	 		{
	 			printf("motar client i er lik: %d\n", i);
                //if position is empty
	 			if( client_socket[i][0] == 0)
	 			{
	 				client_socket[i][0] = new_socket;
	 				printf("Adding to list of sockets as %d\n" , i);
	 					//legger inn portnr
	 				int ret = read(new_socket, &client_socket[i][1], sizeof(int)); //trenger ikke inttypes pga unixsocket
	 				if (ret == 0) 
	 				{
	 					perror("read");
	 					return -7;
	 				}

	 					//0 for client 1 for server
					int retu = read(new_socket, &client_socket[i][2], sizeof(int)); //trenger ikke inttypes pga unixsocket
					if (retu == 0) 
					{
						perror("read");
						return -7;
					}

					if (client_socket[i][2])
						printf("New Local Server has connected\n");
					else
						printf("New Local Client has connected\n");
					break;
				}
			}
		}
		if (FD_ISSET(dsock, &readfds))
		{
			printf("melding fra daemon\n");
			uint32_t dsize = 0;
			int ret = read(dsock, &dsize, sizeof(dsize));
			if ( ret < 0) 
			{
				printf("ret < 0\n");
				perror("read");
				return -5;
			} else if (ret == 0)
			{
				printf("retur == null\n");
				close(dsock);
				FD_CLR(dsock, &fds);
				hide = 1;
			} else
			{
				printf("size er lik: %d\n", dsize);
				buffer = malloc(dsize); //+1 ?
				ret = readfunction(dsock, buffer, dsize);
				printf("strlen buffer uten cast %zd\n", strlen(buffer));
				printf("ret %d\n", ret);
				struct miptp_frame *protocol = (struct miptp_frame*)buffer;
				struct clientheader *protoclient = (struct clientheader*)protocol->file;

				printf("protocol->port: %d\n", protocol->port);
				printf("protocol->psn: %d\n", protocol->psn);
				printf("protoclient size %d\n", protoclient->size);


				if (protoclient->size == 0)
				{
					printf("\nMOTAR ACK!\n\n");
					int k;
					for (k = 0; k < max_clients; k++)
					{
						if (hoved[k].port == protocol->port)
						{
							printf("port stemmer i ack\n");
							if (hoved[k].ack == protocol->psn)
							{
								hoved[k].ack++;
								printf("hoved[k].ack %d\n", hoved[k].ack);
								if (hoved[k].ack < hoved[k].cnt)
								{
									int count = gobackN(hoved[k].qued[hoved[k].ack].buf, dsock, hoved[k].qued[hoved[k].ack].size, hoved[k].port, hoved[k].dst, hoved[k].src, hoved[k].qued[hoved[k].ack].psn, hoved[k].qued[hoved[k].ack].pad);
									printf("count %d\n", count);
									break;
								}
							}
						}
					}
				} else
				{
					for (i = 0; i < max_clients; i++)
					{
						printf("port %d\n", client_socket[i][1]);
						if (client_socket[i][1] == protocol->port)
						{
							if (protocol->psn == client_socket[i][3])
							{
								printf("melding motatt\n");
								client_socket[i][3]++;
								printf("sender ACK\n");
								struct clientheader *ack = malloc(sizeof(struct clientheader));
								bzero(ack, sizeof(struct clientheader));
								ack->src = argv[1][0];
								ack->dst = protoclient->src;
								ack->port = protoclient->port;
								ack->size = 0;
								miptph(ack, dsock, sizeof(struct clientheader), protocol->psn, 0);

								printf("skriver til client på socket %d, og port %d", client_socket[i][0], client_socket[i][1]);
								writefunction(client_socket[i][0], protocol->file, ret - sizeof(struct miptp_frame) - protocol->pl);
								break;
							}
						}
					}
				}
	 		//Sender til port
				//free(buffer);
			}
		}

        //En eksisterende socket sender fil
		for (i = 0; i < max_clients; i++)
		{
			sd = client_socket[i][0];
			if (FD_ISSET(sd, &readfds))
			{
				printf("melding fra eksisterende socket\n");
                //Leser inkommende melding

				valread = read(sd, &siz, sizeof(siz));
				printf("størrelse: %u\nvalread: %u\n", siz, valread);

			  //Socket lukket
				if (valread == 0) 
				{
                    //Lukker socket og setter liste verdi til 0 for gjennbruk
					close(sd);
					client_socket[i][0] = 0;
					client_socket[i][1] = 0;
					client_socket[i][2] = 0;
				} else
				{
					printf("else etter valread\n");
					buffer = malloc(siz);
					bzero(buffer, siz);
					int ret = readfunction(sd, buffer, siz);
					if (client_socket[i][2] == 0) //0 == client, 1 == server
					{
						printf("lest: %d bytes. ment lest: %d\n", ret, siz);
						struct clientheader *protocol = (struct clientheader*)buffer;
						if (!hide)
						{
							int tsiz = siz - sizeof(struct clientheader);
							int cnt = tell(siz - sizeof(struct clientheader));
							int k = 0;
							int count = 0;
							struct que pack[siz];
								//pack = malloc(cnt*sizeof(struct que *));

							printf(" gang %d\n", k);
							del(pack, protocol->buf, tsiz, cnt);


							hoved[i].qued = malloc(tsiz + cnt*sizeof(struct que)+ cnt);
							memset(hoved[i].qued, '\0', tsiz + cnt*sizeof(struct que));
							hoved[i].ack = 0;
							hoved[i].port = protocol->port;
							hoved[i].cnt = cnt;
							hoved[i].dst = protocol->dst;
							hoved[i].src = protocol->src;
							hoved[i].size = tsiz + cnt*sizeof(struct que);

							printf("hoved[i].qued %p %p pack %p %p\n", hoved[i].qued, &hoved[i].qued, pack, &pack);

							printf("pack %d\n", tsiz);

							memcpy(hoved[i].qued, pack, tsiz + cnt*sizeof(struct que)+cnt);

							k = 0;
							if (window <= cnt)
							{
								while(k < window)
								{
									printf("go back n pack[k].size %d k er lik %d\n", pack[k].size, k);
									count = gobackN(pack[k].buf, dsock, pack[k].size, hoved[i].port, hoved[i].dst, hoved[i].src, pack[k].psn, pack[k].pad);
									k++;
								}
							}
							else
							{
								while(k < cnt) 
								{
									printf("go back n pack[k].size %d k er lik %d\n", pack[k].size, k);
									count = gobackN(pack[k].buf, dsock, pack[k].size, hoved[i].port, hoved[i].dst, hoved[i].src, pack[k].psn, pack[k].pad);
									k++;
								}
							}
							printf("count, %d\n", count);

						}
						else
						{
							printf("Ikke tilkoblet daemon, kaster fil\n");
							//splitt(protocol, dsock, protocol->port, siz);

							int tsiz = siz - sizeof(struct clientheader);
							int cnt = tell(siz - sizeof(struct clientheader));
							int k = 0;
							int count = 0;
							struct que pack[siz];
								//pack = malloc(cnt*sizeof(struct que *));

							printf(" gang %d\n", k);
							del(pack, protocol->buf, tsiz, cnt);


							hoved[i].qued = malloc(tsiz + cnt*sizeof(struct que)+ cnt);
							memset(hoved[i].qued, '\0', tsiz + cnt*sizeof(struct que));
							hoved[i].ack = 0;
							hoved[i].port = protocol->port;
							hoved[i].cnt = cnt;
							hoved[i].dst = protocol->dst;
							hoved[i].src = protocol->src;
							hoved[i].size = tsiz + cnt*sizeof(struct que);

							printf("hoved[i].qued %p %p pack %p %p\n", hoved[i].qued, &hoved[i].qued, pack, &pack);

							printf("pack %d\n", tsiz);

							memcpy(hoved[i].qued, pack, tsiz + cnt*sizeof(struct que)+cnt);

							k = 0;
							if (window <= cnt)
							{
								while(k < window)
								{
									printf("go back n pack[k].size %d k er lik %d\n", pack[k].size, k);
									count = gobackN(pack[k].buf, dsock, pack[k].size, hoved[i].port, hoved[i].dst, hoved[i].src, pack[k].psn, pack[k].pad);
									k++;
								}
							}
							else
							{
								while(k < cnt) 
								{
									printf("go back n pack[k].size %d k er lik %d\n", pack[k].size, k);
									count = gobackN(pack[k].buf, dsock, pack[k].size, hoved[i].port, hoved[i].dst, hoved[i].src, pack[k].psn, pack[k].pad);
									k++;
								}
							}
							printf("count, %d\n", count);
						}
					} 
					//free(buffer);
				}
			}
		}
	}
	return 0;
}
