CC = gcc
CFLAGS = -Wall -Wextra -O2 
LFLAGS = -L lib
LFLAGS += -lraylib -lm -lGL

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, src/obj/%.o, $(SRCS))

all: jpaint

src/obj/%.o: src/%.c 
	$(CC) $(CFLAGS) -c -o $@ $< $(LFLAGS)
	

jpaint: $(OBJS)
	$(CC) $(CFLAGS) -o bin/$@ $^ $(LFLAGS)

run: jpaint
	bin/jpaint

clean: 
	$(CC) -o bin/jpaint $(SRCS) $(LFLAGS)
