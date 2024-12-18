CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lzmq

all: controller worker

controller: controller.c
	$(CC) $(CFLAGS) -o controller controller.c $(LDFLAGS)

worker: worker.c
	$(CC) $(CFLAGS) -o worker worker.c $(LDFLAGS)

clean:
	rm -f controller worker

run:
	./controller