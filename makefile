CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -O2

BIN=CS361-main
SRC=main.c ui.c events.c orders.c csvutil.c

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

clean:
	rm -f $(BIN)
