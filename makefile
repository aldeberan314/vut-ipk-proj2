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

argparser.o: server/argparser.cpp
	$(CC) -c server/argparser.cpp

server.o: server/server.cpp
	$(CC) -c server/server.cpp

error.o: utils/error.cpp
	$(CC) -c utils/error.cpp

utils.o: utils/utils.cpp
	$(CC) -c utils/utils.cpp

main.o: server/main.cpp
	$(CC) -c server/main.cpp

client.o: client/client.cpp
	$(CC) -c client/client.cpp

cmain.o: client/cmain.cpp
	$(CC) -c client/cmain.cpp

argparser_client.o: client/argparser_client.cpp
	$(CC) -c client/argparser_client.cpp

run:
	./ipk-simpleftp-server -i en0 -p 5060 -u /Users/andrejhyros/CLionProjects/sftp/debug/userpass.txt -f /Users/andrejhyros/Desktop/tcp

runc:
	./ipk-simpleftp-client -h ::1 -p 5060 -f /Users/andrejhyros/Desktop/tcp

clean:
	rm -f ipk-simpleftp-server ipk-simpleftp-client *.o

clob: # cleans .o files
	rm -f *.o
