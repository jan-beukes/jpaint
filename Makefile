CC = gcc
CFLAGS = -Wall -Wextra
LFLAGS = -lraylib -lm

SRCS = src/*.c

all: main

main: $(SRCS)
	$(CC) -o $@ $^ $(LFLAGS)

run: main
	./main