# Makefile
# Project: Rubel Language
# Derek Tan

# compiler vars
CC := gcc -std=c11
CFLAGS := -Wall -Werror

# executable dir
BIN_DIR := ./bin

# build files dir
BUILD_DIR := ./build

# implementation code dir
SRC_DIR := ./src

# header include dir
HEADER_DIR := ./headers

# auto generate object file target names
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# executable generate path
EXE := $(BIN_DIR)/rubel

vpath %.c $(SRC_DIR)

.PHONY: tell all clean

# utility rule: show SLOC
sloc:
	@wc -l headers/**/*.h src/*.c

# debug rule: show all targets and deps
tell:
	@echo "Code:"
	@echo $(SRCS)
	@echo "Objs:"
	@echo $(OBJS)

# build rules: compiles code and links object files into executable
all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -I$(HEADER_DIR) -o $@

clean:
	rm -f $(EXE) $(BUILD_DIR)/*.o
