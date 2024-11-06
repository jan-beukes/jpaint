CC = gcc
MINGW_CC = x86_64-w64-mingw32-gcc
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

windows:
	$(MINGW_CC) -Wall -o bin/jpaint src/*.c -I windows/include -L windows -lraylib -lm -lgdi32 -lwinmm -lopengl32

run: jpaint
	bin/jpaint

clean: 
	$(CC) -o bin/jpaint $(SRCS) $(LFLAGS)
