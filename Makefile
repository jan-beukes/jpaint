CC = gcc
CFLAGS = -Wall
LFLAGS = lib/libraylib.a -lm -lGL #-lpthread -ldl -lrt -lX11

SRCS = src/*.c

all: jpaint

jpaint: $(SRCS)
	$(CC) $(CFLAGS) -o bin/$@ $^ $(LFLAGS)

run: jpaint
	bin/jpaint

clean: 
	$(CC) -o bin/jpaint $(SRCS) $(LFLAGS)