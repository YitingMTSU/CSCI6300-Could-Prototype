CC = gcc
CFLAGS =
DEPS = server.h
OBJ = server.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm server
	rm *.o
	rm *.~
