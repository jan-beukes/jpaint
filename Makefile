CC = gcc
CFLAGS = -Wall -Wextra
LFLAGS = -lraylib -lm # -lGL -lpthread -ldl -lrt -lX11

SRCS = src/*.c

all: paint

paint: $(SRCS)
	$(CC) -o $@ $^ $(LFLAGS)

run: paint
	./paint