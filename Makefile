.RECIPEPREFIX = >
CC     = gcc
CFLAGS = -Wall -Wextra -pthread -g
TARGET = game

all: $(TARGET)

$(TARGET): src/game.c
> $(CC) $(CFLAGS) -o $@ $<

run: all
> ./$(TARGET)

test: all
> bash tests/test.sh

monitor: all
> bash scripts/run_game.sh

clean:
> rm -f $(TARGET)
> rm -f logs/*.log

.PHONY: all run test monitor clean
