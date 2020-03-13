CC = gcc
CFLAGS = 
DEPS = client.h
OBJ = client.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
client: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm client
	rm *.o
	rm *.~
