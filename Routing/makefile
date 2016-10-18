#CC = Konstant
CC=gcc 
# Legger alle filene inn i Filer
FILER=client.c server.c daemon.c "send&recv.c" functions.c cashe.c raw.c
DAEMON = daemon.c "send&recv.c" functions.c cashe.c raw.c
CLIENT = client.c "send&recv.c" functions.c 
SERVER = server.c "send&recv.c"
#setter navn på ferdigkompilert fil
UTFIL1= m.o
UTFIL2= c.o
UTFIL3= s.o

all: komp

#kjører dersom komandoen make blir brukt
komp: 
	@$(CC) -o $(UTFIL1) $(DAEMON)
	@$(CC) -o $(UTFIL2) $(CLIENT)
	@$(CC) -o $(UTFIL3) $(SERVER)
	@echo $(FILER) kompilert

	@echo kjør med ./$(UTFIL1) [HOST]
	@echo så i ny terminal med ./$(UTFIL2) [HOST][DESTINATION][MSG]
	@echo server = ./$(UTFIL3) [HOST]

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
	@echo kjør med ./$(UTFIL1) [HOST]

#make serv for å kompilere server 
serv: 
	@$(CC) -o $(UTFIL2) $(SERVER)
	@echo $(SERVER) kompilert
	@echo kjør med ./$(UTFIL2) [HOST]
	
#make cli for å kompilere client
cli:
	@$(CC) -o $(UTFIL1) $(CLIENT)
	@echo $(CLIENT) kompilert
	@echo kjør med ./$(UTFIL1) [HOST][DESTINATION][MSG]

