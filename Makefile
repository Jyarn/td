CC=gcc
LFLAGS=-lasan
CFLAGS=-fsanitize=address -g -Wall -Wextra -Wpedantic
EXE=td

HEAD=all.h
SRC=main.c display.c items.c templates/item-pool.c

.PHONY: clean build metagen

build: $(EXE)

dbg: $(EXE)
	gdb $(EXE) -x .gdbscript

run: $(EXE)
	./$(EXE)

$(EXE): $(addsuffix .o, $(addprefix bin/, $(basename $(SRC)))) $(wildcard shaders/*)
	$(CC) $(LFLAGS) $(filter %.o,$^) -o $@

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

metagen:
	./lib/templates/pool-allocator/import.sh --output src/templates --struct-prefix Item --func-prefix item_pool --elem-type Item --file-name item-pool

clean:
	-rm $(EXE) bin/*
	-rm bin/templates/*
