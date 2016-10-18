//Deklarerer metoder som finnes andre steder
/*send&recv.c*/ //Metoder for å lese og skrive 
/**
*tar inn parametere til read/write
*returnerer hvor mye som er lest/skrevet
*/
int readfunction(int socket, char *buf, int8_t size);
int writefunction(int socket, char *buffer, int8_t siz);

/*functions.c*/
char* transport(char host, char recv, int size); //Disse metodene setter bit tilsvarende sin oppgave
char* transports(int size);						 //Tar inn blandt annet mip adresser
char* routing(char from);						//returnerer header eller deler av header
char* broadcast(char host);
char* broadcasts(void);
char* response(char host, char recv);
char* responses();

/**
*tar inn char som inneholder tra. regner om fra bit verdi til desimal og returnerer
*/
int traNum(char *c);
/**
*metode for å opphøye 2 i a
*2 opphøyes a antall ganger før resultatet returneres.
*/
int ganger(int a);
int payload_length(char *c);

/*cache.c*/
/**
*miparpcache struct. inneholder mip, mac og en peker til neste element
*brukes til oppslag og lagring av mac
*/
typedef struct miparpcache 
{
	char mipadr;
	uint8_t mac[6];
	int socket;
	struct miparpcache *next; //nestepeker for å senere ha en enkel listeløsning, slippe array
} cache;

/* Struct for interfacene */
typedef struct interfaces {
  char name[7];
  struct interfaces *next;
} iface;

/*Struct for routingtabell*/
struct routing 
{
  char host; //tabelleier
  uint8_t length; //lenght of path
  char dest; //destination of route
  char rout; //forward address
}__attribute__((packed));


void addcache(cache **root, char mip, uint8_t mac[6], int socket); //tar inn en node, en mip og en mac
char retmip(struct miparpcache **root,  uint8_t mac[6]);//brukes ikke, for å sende inn en mac og hente ut en mip
int retmac(cache **root,  char mipadr, uint8_t mac[6]); //Sender inn en mip og finner tilhørende mac i structen
int nexthop(char dst, struct routing rt[4]);
void printmac(uint8_t mac[6]); //printer mac
void printinterfaces(iface **face);
void printCache(cache **root); //tar inn rotnoden printer alle mip og mac. 

/* raw.c */
 struct ether_frame //definisjon av en ethernet frame
 {
 	uint8_t dst_addr[6];
 	uint8_t src_addr[6];
 	uint8_t eth_proto[2];
 	char contents[0];
 } __attribute__((packed));

 /*Struct for MIP Header*/

struct mip
{

	unsigned char tra : 3;
	unsigned char ttl : 4;
	unsigned int payload : 9;
	char src;
	char dst;
	char buf[0];

} __attribute__((packed));

/*Struct for timeout skjekk av nabo node*/
struct link_timeout {
  time_t time;
  char mip;
};


/*oppdaterer link timeout*/
/**
*Tar inn en timeout, en mip og en størrelse. 
*Oppdaterer tiden hvis mip adressen er lagret i tabellen
*Eventuelt lagrer mip først, så oppdaterer tid
*/
void update_time(struct link_timeout *l_table, char mip, int size);

/**
*Metode for å skjekke om en mip adresse finnes i timeout tabellen
*/
int link_table_check(struct link_timeout *l_table, char mip, int size);
/**
*Metode for å sette verdier i routingtabellen til 0, så routing tabellen ikke påvirker en nabo med
*med info den har fått av gitt nabo.
*/
void split_horizon(struct routing rt[4], struct routing rt_horizon[4], char mip);
/**
*Metode for å fjerne node fra routing tabell og tidstabell
*Fjerner dersom koblingen med en mip adresse er mistet
*/
void bortfallendenode(char mipadr, struct routing rt[4], struct link_timeout *l_table, int size);

/**
*Metode for å printe ut routing tabellen
*/
void printroutingtabell(char mipadr, struct routing rt[4]);


/**
*finner fram interface
*returnerer feks eth0
*/
int get_interface(iface **inter); 

/**
*tar in source mip, destination mip, socket host macadresse destination mac adresse, en melding
*og et tall som bestemmer hva som skal gjøres. 
*lager mip header og en ethernet frame
*sender over rawsocket
*/
void arp(struct routing *rc, char src, char dst, int rawsock, uint8_t hmac[6], uint8_t dmac[6], char* buf, int mode, int ttl);

/**
*finner local mac adresse
*sender inn socket, interface og en adresse til en uint8_t[6] som macen blir lagt i
*returnerer 0 på suksess, -1 om ioctl kallet feiler
*/
int get_if_hwaddr(int sock, const char* devname, uint8_t hwaddr[6]); 