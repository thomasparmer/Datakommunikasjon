#CC = Konstant
CC=gcc 
# Legger alle filene inn i Filer
FILER=client.c server.c daemon.c "send&recv.c" functions.c cashe.c raw.c miptp.c transferserver.c transferclient.c que.c local.c
DAEMON = daemon.c "send&recv.c" functions.c cashe.c raw.c
CLIENT = transferclient.c "send&recv.c" functions.c 
SERVER = transferserver.c "send&recv.c"
MIPTP = miptp.c local.c que.c "send&recv.c"
#setter navn på ferdigkompilert fil
UTFIL1= m.o
UTFIL2= c.o
UTFIL3= s.o
UTFIL4= t.o

all: komp

#kjører dersom komandoen make blir brukt
komp: 
	@$(CC) -o $(UTFIL1) $(DAEMON)
	@$(CC) -o $(UTFIL2) $(CLIENT)
	@$(CC) -o $(UTFIL3) $(SERVER)
	@$(CC) -o $(UTFIL4) $(MIPTP)
	@echo $(FILER) kompilert

	@echo ./$(UTFIL1) [socket]
	@echo ./$(UTFIL2) [socket][filename][Destination Adress][port]
	@echo ./$(UTFIL3) [socket][filename][size][port]
	@echo ./$(UTFIL4) [socket][timeout]

#For debuging
debug: CC += -DDEBUG -g -Wall
debug: komp	

#make clean for å kjøre. Fjerner alle .o filer
clean: 
	@rm -r *.o
	@echo Clean done

#make rekomp for å kjøre clean, derretter komp
rekomp: clean komp
	
#make daem for å kompilere
daem: 
	@$(CC) -o $(UTFIL1) $(DAEMON)
	@echo $(DAEMON) kompilert
	@echo ./$(UTFIL1) [HOST]

#make serv for å kompilere server 
serv: 
	@$(CC) -o $(UTFIL3) $(SERVER)
	@echo $(SERVER) kompilert
	@echo ./$(UTFIL3) [filename][size][port]
	
#make cli for å kompilere client
cli:
	@$(CC) -o $(UTFIL2) $(CLIENT)
	@echo $(CLIENT) kompilert
	@echo ./$(UTFIL2) [filename][Destination Adress]

miptp:
	@$(CC) -o $(UTFIL4) $(MIPTP)
	@echo $(MIPTP) kompilert
	@echo ./$(UTFIL4) [socket]