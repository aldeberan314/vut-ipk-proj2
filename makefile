CC=g++
LBS=-lpcap
OBJS=argparser.o server.o error.o utils.o main.o
APPNAME=sftp

all: sftp clean_objs

sftp: $(OBJS)
	$(CC) -o $(APPNAME) $(OBJS)

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

run:
	./sftp -i en0 -p 117 -u cesta -f cestam

clean:
	rm -f sftp *.o

clean_objs:
	rm -f *.o