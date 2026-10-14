.DEFAULT_GOAL := all

SRCS = main.c game_logic.c

ifeq ($(OS),Windows_NT)
    CC = gcc
    TARGET = game.exe
    LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
else
    CC = cc
    TARGET = game
    CFLAGS = $(shell pkg-config --cflags raylib)
    LIBS = $(shell pkg-config --libs raylib)
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

# Prevent Makefile regeneration
Makefile: ;

clean:
	rm -f $(TARGET)