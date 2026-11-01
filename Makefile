CC=gcc
LFLAGS=-lasan
CFLAGS=-fsanitize=address -g -Wall -Wextra -Wpedantic
EXE=fm

HEAD=all.h
SRC=main.c

.PHONY: clean build

build: $(EXE)

dbg: $(EXE)
	gdb $(EXE) -x .gdbscript

run: $(EXE)
	./$(EXE)

$(EXE): $(addsuffix .o, $(addprefix bin/, $(basename $(SRC)))) $(wildcard shaders/*)
	$(CC) $(LFLAGS) $(filter %.o,$^) -o $@

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm $(EXE) bin/*
