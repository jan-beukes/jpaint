CC = gcc
CFLAGS = -Wall -Wextra
LFLAGS = -lraylib -lm

SRCS = src/*.c

all: paint

paint: $(SRCS)
	$(CC) -o $@ $^ $(LFLAGS)

run: paint
	./paint