CC=mpicc
RUN=mpirun
AGENTS=4
BUILD_DIR=build

CFLAGS=-Wall -std=gnu99

SRC=$(wildcard *.c)
EXE=$(SRC:.c=)

TO_RUN?=hello_world
ARGS?=

all: $(EXE)

clean:
	rm -rf $(BUILD_DIR)/*

run: $(TO_RUN)
	$(RUN) -np $(AGENTS) $(BUILD_DIR)/$< $(ARGS)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $@.c