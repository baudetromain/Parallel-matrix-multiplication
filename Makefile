CC=mpicc
RUN=mpirun
AGENTS?=4
BUILD_DIR=build

CFLAGS=-Wall -std=gnu99

all:
	$(CC) $(CFLAGS) matrix_multiplication_better.c -o $(BUILD_DIR)/matrix_multiplication_better

clean:
	rm -rf $(BUILD_DIR)/*