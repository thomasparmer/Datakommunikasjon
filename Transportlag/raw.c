#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <linux/if_link.h>   
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <bits/ioctls.h>

#include "functions.h"

/**
 * struct definisjon av ethernet frame
 */ 

/**
 * Henter mac adresse (hardware adresse)
*/ /*static*/
 int get_if_hwaddr(int rawsock, const char* devname, uint8_t hwaddr[6])
 {
 	struct ifreq ifr;
 	memset(&ifr, 0, sizeof(ifr));

 	assert(strlen(devname) < sizeof(ifr.ifr_name));
 	strcpy(ifr.ifr_name, devname);

 	if(ioctl(rawsock, SIOCGIFHWADDR, &ifr) < 0)
 	{
 		perror("ioctl");
 		return -1;
 	}

 	memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, 6*sizeof(uint8_t));

 	return 0;
 }

/* finner interface*/
 int get_interface(iface **inter) {
  struct ifaddrs *ifaddr, *ifa;
  int family, name, i;
  char host[NI_MAXHOST];
  int cnt = 0;

  if(getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    return 0;
  }
  for (ifa = ifaddr, i = 0; ifa != NULL; ifa = ifa->ifa_next, i++) 
  {
    if (ifa->ifa_addr == NULL)
      continue;
    
    family = ifa->ifa_addr->sa_family;
    
    if (family == AF_INET6) 
    {
      name = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      
      if (name != 0) {
       perror("getnameinfo");
       return 0;
     }

     if(strcmp(ifa->ifa_name, "lo") != 0) 
     {
      cnt++;
      if(!*inter)
      {
        *inter = malloc(sizeof(struct interfaces));
        strcpy((*inter)->name, ifa->ifa_name);
      } else
      {
        iface *tmp = malloc(sizeof(struct interfaces));
        strcpy(tmp->name, ifa->ifa_name);
        tmp->next = *inter;
        *inter = tmp;
      }
    }
  }
}
freeifaddrs(ifaddr);
return cnt;
}

void arp(char src, char dst, int rawsock, uint8_t hmac[6], uint8_t dmac[6], /*char* buf*/struct miptp_frame *buf, int tra, int size)
{
printf("mottat fra local i raw tidlig: %d\n", size);
  size_t siz;
  ssize_t header_size = 0;
  struct mip *protocol;
  if (buf)
    siz = (sizeof(struct mip) + size);
  else
    siz = sizeof(struct mip);
  protocol = malloc(siz);
  bzero(protocol, siz);
  /**
  *mip protocol arp
  */

  protocol->tra = tra;
  protocol->ttl = 15;
  if (buf)
    protocol->payload = (size/4);
  else
    protocol->payload = 0;

  protocol->src = src;
  if (dst != '\0')
    protocol->dst = dst;

  if (buf)
  {
   memcpy(protocol->buf, buf, size);

   /* memcpy(protocol->buf, &buf->port, sizeof(int));
    memcpy(protocol->buf+4, buf->psn, 2);
    memcpy(protocol->buf+6, buf->file, size-sizeof(struct miptp_frame));
*/

   //fprintf(stderr, "protocol size: %zd, siz: %zd melding: %s lengde buf:%zd\n", sizeof(protocol), siz, protocol->buf, strlen(protocol->buf));
/*
   struct miptp_frame  *tp2 = (struct miptp_frame*)protocol->buf;
   struct clientheader *ch = (struct clientheader*)tp2->file;
   printf("protocol port raw daemon: %d\n", tp2->port);
   printf("protocol raw daemon: %s\n", ch->buf);

*/

 }


  /*Making a raw ethernet fram and allocate space*/
 printf("full header %zd\n", ((sizeof(struct ether_frame)) + sizeof(struct mip) + size));

 header_size = (sizeof(struct ether_frame)) + sizeof(struct mip) + size;
 printf("motatt fra local i raw %zd\n", size-sizeof(struct ether_frame) + sizeof(struct mip));

 struct ether_frame *frame = malloc(header_size);
 bzero(frame, header_size);
 assert(frame);

  /*Filling the frame for sending*/

  /*Broadcast address*/
 if(tra == 1 || tra == 2)
    //memcpy(frame->dst_addr, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
  memset(frame->dst_addr, 255, 6);



if(tra == 0 || tra == 4)
  memcpy(frame->dst_addr, dmac, 6);

  /*Source address*/
memcpy(frame->src_addr, hmac, 6);

  /*Protocol field*/
frame->eth_proto[0] = frame->eth_proto[1] = 0xFF;

  /*Sets the header*/
memcpy(frame->contents, protocol, sizeof(struct mip) + size);
 //fprintf(stderr, "frame->dst_addr": );

/*
  printmac(frame->dst_addr);
  printmac(frame->src_addr);
  fprintf(stderr, "ethproto: %d %d \n", frame->eth_proto[0], frame->eth_proto[1]);
  fprintf(stderr, "tra %d\n", ((struct mip*)(frame->contents))->tra);
  fprintf(stderr, "ttl %d\n", ((struct mip*)(frame->contents))->ttl);
  fprintf(stderr, "payload %d\n", ((struct mip*)(frame->contents))->payload);
  fprintf(stderr, "src %c\n", ((struct mip*)(frame->contents))->src);
  fprintf(stderr, "dst %c\n", ((struct mip*)(frame->contents))->dst);
  fprintf(stderr, "buf %s\n", ((struct mip*)(frame->contents))->buf);
*/

  struct sockaddr rawaddr;
  rawaddr.sa_family = AF_PACKET;

  memcpy(&rawaddr.sa_data, &frame->dst_addr, 6);

  int se = write(rawsock, frame, header_size);
  if (se == -1)
    perror("send raw socket\n");

/*
  if (buf)
  {
    struct mip *m = (struct mip*)frame->contents; 
    struct miptp_frame  *tp = (struct miptp_frame*)m->buf;
    struct clientheader *ch = (struct clientheader*)tp->file;
    printf("ch -> buf i raw daemon: %s\n", ch->buf);
  }
  */




#ifdef DEBUG

  if(tra == 1)
    printf("ARP Broadcast sent med meldingsstørrelse %zu\n", header_size);

  if(tra == 0)
    printf("ARP Response sent med meldingsstørrelse %zu\n", header_size);

  if(tra == 4)
    printf("Transport sent med størrekse %zu\n bytes skrevet %d\n", header_size, se);

  if (tra == 2)
    printf("Routing sent med størrelse %zu\n", header_size);
#endif
}
