CC = gcc
CFLAGS = -Wall -Iinclude

# Directories
SRC_DIR = src
PARSER_DIR = parser
SERVER_DIR = server
UTILS_DIR = utils
INCLUDE_DIR = include

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
PARSER_FILES = $(wildcard $(PARSER_DIR)/*.c)
SERVER_FILES = $(wildcard $(SERVER_DIR)/*.c)
UTILS_FILES = $(wildcard $(UTILS_DIR)/*.c)

# Object files
OBJ_FILES = $(SRC_FILES:.c=.o) $(PARSER_FILES:.c=.o) $(SERVER_FILES:.c=.o) $(UTILS_FILES:.c=.o)

# Targets
all: main

main: $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_FILES) main
