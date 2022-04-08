CC=g++ -std=c++17
LBS=-lpcap
S_OBJS=argparser.o server.o error.o utils.o main.o
C_OBJS=client.o error.o utils.o cmain.o
SERVERNAME=ipk-simpleftp-server
CLIENTNAME=ipk-simpleftp-client

all: server client

client: $(C_OBJS) clean_objs
	$(CC) -o $(CLIENTNAME) $(C_OBJS)

server: $(S_OBJS) clean_objs
	$(CC) -o $(SERVERNAME) $(S_OBJS)

argparser.o: argparser.cpp
	$(CC) -c argparser.cpp

server.o: server.cpp
	$(CC) -c server.cpp

error.o: error.cpp
	$(CC) -c error.cpp

utils.o: utils.cpp
	$(CC) -c utils.cpp

main.o: main.cpp
	$(CC) -c main.cpp

client.o: client.cpp
	$(CC) -c client.cpp

cmain.o: cmain.cpp
	$(CC) -c cmain.cpp






run:
	./ipk-simpleftp-server -i en0 -p 117 -u cesta -f cestam

clean:
	rm -f ipk-simpleftp-server ipk-simpleftp-client *.o

clean_objs:
	rm -f *.o