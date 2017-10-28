CC=g++
CFLAGS=-I
DEPS = common.h
EXEC = client server
all: $(EXEC)

client: bclient_basic.cpp $(DEPS)
	$(CC) -o client bclient_basic.cpp
server: bserver.cpp $(DEPS)
	$(CC) -o server bserver.cpp -lpthread
clean:
	rm $(EXEC) 
