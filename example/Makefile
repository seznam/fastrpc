CXX=g++
CFLAGS=-Wall
LDFLAGS=-lfastrpc -lxml2


all: server client

server: server_example.o
	$(CXX)  server_example.o    -o server $(LDFLAGS)


client: client_example.o
	$(CXX)  client_example.o    -o client $(LDFLAGS)

clean: 
	rm -f *.o client server 


