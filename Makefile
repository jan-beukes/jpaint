CC = gcc
CFLAGS = -Wall -Wextra
LFLAGS = -lraylib -lm -lGL #-lpthread -ldl -lrt -lX11

SRCS = src/*.c

all: jpaint

jpaint: $(SRCS)
	$(CC) -o $@ $^ $(LFLAGS)

run: jpaint
	./jpaint

clean: 
	$(CC) -o jpaint $(SRCS) $(LFLAGS)