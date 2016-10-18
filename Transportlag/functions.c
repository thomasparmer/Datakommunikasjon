#include <stdio.h>
#include <stdlib.h>

//finner headere

/*
Setter heading for transport
tra blir lik 100
*/

/*Struct for MIP Header*/
struct mip_frame {
  char tra[2];
  char src;
  char dst;
  char message[0];
}__attribute__((packed));


char* transport(char host, char recv, int size) 
{
  char *mip = malloc(4);

  mip[0] |= 1<<7;  
  mip[2] = host;
  mip[3] = recv;
  
  int dec = size;
  int block = 1;
  int counter = 0;

  while(dec != 0) 
  {

    if(dec%2 == 1) 
    {
      mip[block] |= (1<<counter);
    }
    counter++;
    dec = dec/2;

    if(counter == 8) 
    {
      block = 0;
      counter = 0;
    }
  }
  return mip;
}

char* transports(int size) 
{
  char *mip = malloc(2);
  mip[0] |= 1<<7; //setter tra sekvens 100  
  int s = size;
  int block = 1;
  int counter = 0;
  
  while(s != 0) {
    if(s%2 == 1)
      mip[block] |= (1<<counter);
    counter++;
    s = s/2;
      
    if(counter == 8) {
      block = 0;
      counter = 0;
    }
  }
  return mip;
}



/*
metode for routing
setter header for routing..kommer ikke til å bli brukt
setter tra til 010

*/
char* routing(char from) 
{
  char *mip = malloc(4);
  mip[2] = from;
  int i;
  mip[1] = 0;
  mip[0] |= 1<<6;
  for(i = 0; i <= 4; i++) 
  {
    mip[0] |= 1<<i;
  }
  for(i = 0; i <= 7; i++) 
  {
    mip[3] |= 1<<i;
  }
  return mip;
}

/*
Metode for broadcast
setter tra til å være 001
*/

char* broadcast(char host) 
{
  char *mip = malloc(4);
  mip[2] = host;
  int i;
  mip[1] = 0;
  mip[0] |= 1<<5;
  for(i = 0; i <= 4; i++) 
  {
    mip[0] |= 1<<i;
  }
  for(i = 0; i <= 7; i++) 
  {
    mip[3] |= 1<<i;
  }
  return mip;
}

/*
Metode for broadcast, tar ikke inn noe mallocer 2
Setter tra til å være 001
*/
char* broadcasts(void) 
{
  char *mip = malloc(2);
  mip[0] |= 1<<5; //setter tra til 001
  mip[1] = 0;

  int i;

  /*TTL 1111*/
  for(i = 0; i <= 4; i++) {
    mip[0] |= 1<<i;
  }

  return mip;
  
}


/*
Metode for response
Setter tra til å være 000
*/

char* response(char host, char recv) 
{
  char *mip = malloc(4);
  mip[0] = 0;
  mip[1] = 0;
  mip[2] = host;
  mip[3] = recv;

  int i;
  
  for(i = 0; i <= 4; i++) 
  {
    mip[0] |= 1<<i;
  }
  
  return mip;
}
/**
*Setter tra til å være 000
*/
char* responses() 
{
  int i;
  char *mip = malloc(2);
  mip[0] = 0;
  mip[1] = 0;
  
  for(i = 0; i <= 4; i++)
    mip[0] |= 1<<i;
  return mip;
}


//Metode for å hente TRA
int traNum(char *c)
{
  int val;
  int ret = 0;
  
  val = (c[0]>>5) & 1;
  if (val == 1)
  {
    ret += 1;
  }
  
  val = (c[0]>>6) & 1;
  if (val == 1)
  {
    ret += 2;
  }

  val = (c[0]>>7) & 1;
  if (val == 1)
  {
    ret += 4;
  }
  return ret;
}
/*
metode for å opphøye 2 i a
2 opphøyes a antall ganger før resultatet returneres.
*/

int ganger(int a) 
{
  int counter = 2;
  int res = 2;

  if ( a == 0)
  {
    return 1;
  }
  if (a == 1)
  {
    return 2;
  }

  while (counter <= a)
  {
    res = 2*res;
    counter++;
  }
  return res;
}


/*Metode for payload
gjør tallet om fra binær til decimal


*/

int payload_length(char *c)
{
  int val;
  int ret = 0;
  int counter = 7;

  val = (c[0]>>0) & 1;
  if (val == 1) 
  {
    ret += ganger(8);
  }

  while (counter >= 0)
  {
    val = (c[1]>>counter) & 1;
    if (val == 1)
    {
      ret += ganger(counter);
    }
    counter--;
  }
  return ret;
}

