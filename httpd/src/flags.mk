SRC_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

CC = gcc
CPPFLAGS = -I$(SRC_DIR)
CFLAGS = -Wall -Wextra -Werror -Wvla -std=c99 -pedantic
LDFLAGS = -fsanitize=address -g

AR = ar
ARFLAGS = rcvs
