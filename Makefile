CC = gcc
MINGW_CC ?= x86_64-w64-mingw32-gcc

CFLAGS = -Wall -Wextra -O2 

LFLAGS = -L ./libs
LFLAGS += -lraylib -lm -lGL
IFLAGS = -I ./libs/include 

WFLAGS = -I libs/include -I libs/windows/include -L libs/windows 
WFLAGS += -lraylib -lm -lgdi32 -lwinmm -lopengl32

VERSION = 0.1

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, src/obj/%.o, $(SRCS))

all: jpaint

src/obj/%.o: src/%.c 
	$(CC) $(CFLAGS) -c -o $@ $< $(IFLAGS) 

jpaint: $(OBJS)
	@[ -d bin ] || mkdir -p bin
	$(CC) $(CFLAGS) -o bin/$@ $^ $(LFLAGS)

windows:
	@[ -d bin ] || mkdir -p bin
	$(MINGW_CC) -Wall -o bin/jpaint src/*.c $(WFLAGS)

release: windows
	cp bin/jpaint.exe release
	cd release && zip jpaint-win64-$(VERSION).zip jpaint.exe

run: jpaint
	bin/jpaint

clean: 
	rm -rf src/obj
