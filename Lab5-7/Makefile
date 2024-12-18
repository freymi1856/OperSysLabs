CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lzmq

all: controller compute_node

controller: controller.c
	$(CC) $(CFLAGS) -o controller controller.c $(LDFLAGS)

compute_node: compute_node.c
	$(CC) $(CFLAGS) -o compute_node compute_node.c $(LDFLAGS)

clean:
	rm -f controller compute_node

run:
	./controller