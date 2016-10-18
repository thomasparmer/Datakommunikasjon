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

/*Method for updating the link_timeout*/
void update_time(struct link_timeout *l_table, char mip, int size) 
{

  int i;

  /*Update the time if the mip is stored in the table*/
  if(link_table_check(l_table, mip, size)) 
  {
    for(i = 0; i < size; i++) 
      if(l_table[i].mip == mip) 
        l_table[i].time = time(NULL); 

    /*inserts the mip in the table and updates the time*/
    } else 
    {
      for(i = 0; i < size; i++)
        if(l_table[i].mip == 0) 
        {
          l_table[i].mip = mip;
          l_table[i].time = time(NULL);
          break;
        }
      }
    }

    /*Method for checking l_table contents*/
    int link_table_check(struct link_timeout *l_table, char mip, int size) 
    {
      int i;
      for(i = 0; i < size; i++) 
        if(l_table[i].mip == mip)
          return 1;
        return 0;
      }

      void split_horizon(struct routing rt[4], struct routing horizon[4], char mip) 
      {
        int i;
        for(i = 0; i < 4; i++) {
          if(rt[i].rout != mip) {

            horizon[i].dest = rt[i].dest;
            horizon[i].host = rt[i].host;
            horizon[i].length = rt[i].length;
            horizon[i].rout = rt[i].rout;
          }else {
           horizon[i].dest = 0;
           horizon[i].host = rt[i].host;
           horizon[i].length = 0;
           horizon[i].rout = 0;
         }
       }
     }


/*Method for removing node from routing table and time table*/
     void bortfallendenode(char mipadr, struct routing routingtabell[4], struct link_timeout *linktimeout_table, int size)
     {

      int i;
      for(i = 0; i < 4; i++)
        if(routingtabell[i].dest == mipadr) {
          routingtabell[i].dest = 0;
          routingtabell[i].length = 0;
          routingtabell[i].rout = 0;
        #ifdef DEBUG
          printf("Mistet kobling med mipadr, fjernes fra tabell %c\n", mipadr);
        #endif
        }

        for(i = 0; i < size; i++)
        {
          if(linktimeout_table[i].mip == mipadr)
            linktimeout_table[i].mip = 0;
        }

      }

    /*Finner neste steg i routingtabellen, returnerer arrayindex*/
      int nexthop(char dst, struct routing rt[4]) 
      {
        int i;
        for(i = 0; i < 4; i++) 
          if(rt[i].dest == dst)
          {
            return i;
          }
         return -1; //mip finnes ikke
       }


       void printroutingtabell(char mipadr, struct routing routingtabell[4]) 
       {

        int i;
        printf("Routing tabell for %c\n", mipadr);

        for(i = 0; i < 4; i++)
          if(routingtabell[i].dest != 0)
            printf("Destinasjon: %c \tNestehopp: %c\tstilengde: %d\n", routingtabell[i].dest, routingtabell[i].rout, routingtabell[i].length);

        }


        void arp(struct routing *rc, char src, char dst, int rawsock, uint8_t hmac[6], uint8_t dmac[6], char* buf, int tra, int ttl)
        {

          size_t siz;
          ssize_t header_size;
          struct mip *protocol;
          if (buf)
            siz = (sizeof(struct mip) + strlen(buf));
          else if (rc)
          {
            siz = (sizeof(struct mip) + (4*sizeof(struct routing)));
          }
          else
            siz = sizeof(struct mip);

          protocol = malloc(siz);
          bzero(protocol, siz);
  /**
  *mip protocol arp
  */

  protocol->tra = tra;
  protocol->ttl = ttl;
  if (buf)
    protocol->payload = strlen(buf);
  else if (rc)
    protocol->payload = (4*sizeof(struct routing));
  else
    protocol->payload = 0;

  protocol->src = src;
  if (dst != '\0')
    protocol->dst = dst;

  if (buf)
  {
    memcpy(protocol->buf, buf, strlen(buf));
    printf("protocol size: %zd, siz: %zd melding: %s lengde buf:%zd\n", sizeof(protocol), siz, protocol->buf, strlen(protocol->buf));
  } else if (rc)
  {
    memcpy(protocol->buf, rc, (4*sizeof(struct routing)));
  }



  /*Making a raw ethernet fram and allocate space*/
  header_size = protocol->payload;
  siz = (sizeof(struct ether_frame)) + sizeof(struct mip) + header_size;

  struct ether_frame *frame = malloc(siz);
  bzero(frame, siz);
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
  memcpy(frame->contents, protocol, sizeof(struct mip) + header_size);
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


  int se = write(rawsock, frame, siz);
  if (se == -1)
    perror("send raw socket\n");

#ifdef DEBUG

  if(tra == 1)
    printf("ARP Broadcast sent med meldingsstørrelse %zu\n", siz);

  if(tra == 0)
    printf("ARP Response sent med meldingsstørrelse %zu\n", siz);

  if(tra == 4)
    printf("Transport sent med størrekse %zu\n", siz);

  if (tra == 2)
    printf("Routing sent med størrelse %zu\n", siz);
#endif
}
