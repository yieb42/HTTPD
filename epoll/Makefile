export CC = gcc
export CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic

BINARY=epoll

all: $(BINARY)

$(BINARY): epoll.c
	$(CC) $(CFLAGS) $< -o epoll

clean:
	$(RM) $(BINARY)

.PHONY: all
