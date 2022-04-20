CC=g++ -std=c++17
LBS=-lpcap
S_OBJS=argparser.o server.o error.o utils.o main.o
C_OBJS=client.o error.o utils.o argparser_client.o cmain.o
SERVERNAME=ipk-simpleftp-server
CLIENTNAME=ipk-simpleftp-client

all: server client clob

client: $(C_OBJS)
	$(CC) -o $(CLIENTNAME) $(C_OBJS)

server: $(S_OBJS)
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

argparser_client.o: argparser_client.cpp
	$(CC) -c argparser_client.cpp

run:
	./ipk-simpleftp-server -i en0 -p 5060 -u /Users/andrejhyros/CLionProjects/sftp/userpass.txt -f /Users/andrejhyros/Desktop/tcp

runc:
	./ipk-simpleftp-client -h ::1 -p 5060 -f /Users/andrejhyros/Desktop/tcp

clean:
	rm -f ipk-simpleftp-server ipk-simpleftp-client *.o

clob: # cleans .o files
	rm -f *.o
