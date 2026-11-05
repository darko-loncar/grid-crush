# Name of the executable
TARGET = gridcrush
TEST_TARGET = tests/matrix_test
GAME_TEST_TARGET = tests/tetroid_test

# Compiler
CC = gcc

# Compiler flags (optional)
CFLAGS = -Wall -Wextra -O2 $(shell pkg-config --cflags sdl3 sdl3-ttf)

# Linker flags
LDFLAGS = $(shell pkg-config --libs sdl3 sdl3-ttf) -lm

# Source files
SRC = main.c tetroid.c matrix.c draw.c sound.c ui.c preview.c input.c
HEADERS = tetroid.h matrix.h draw.h color.h sound.h ui.h preview.h input.h

# Build target
$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

debug: $(SRC) $(HEADERS)
	$(CC) -Wall -Wextra -g -O0 $(shell pkg-config --cflags sdl3 sdl3-ttf) $(SRC) -o $(TARGET)-debug $(LDFLAGS)


# Unit tests (no SDL dependency)
$(TEST_TARGET): tests/matrix_test.c tests/pacman_map.txt matrix.c matrix.h
	$(CC) -Wall -Wextra -O2 tests/matrix_test.c matrix.c -o $(TEST_TARGET)

$(GAME_TEST_TARGET): tests/tetroid_test.c tetroid.c tetroid.h matrix.c matrix.h color.h
	$(CC) -Wall -Wextra -O2 tests/tetroid_test.c tetroid.c matrix.c -o $(GAME_TEST_TARGET)

test: $(TEST_TARGET) $(GAME_TEST_TARGET)
	./$(TEST_TARGET)
	./$(GAME_TEST_TARGET)

# Clean target
.PHONY: clean test debug
clean:
	rm -f $(TARGET) $(TARGET)-debug $(TEST_TARGET) $(GAME_TEST_TARGET)
