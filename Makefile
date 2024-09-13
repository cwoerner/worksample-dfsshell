# Variables
CC = gcc
INDENT = /usr/local/opt/gnu-indent/libexec/gnubin/indent
SRC_DIR = src
OBJ_DIR = obj
TARGET = dfsshell

# File pattern for sources and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Compiler flags
# CFLAGS = -Wall -Werror -g
CFLAGS = -g -Wall
CPPFLAGS += '-DDFS_YAML="/opt/dfsshell/dfsshell/dfs.yaml"'

# Rule to link the final executable
$(TARGET): $(OBJ_DIR) $(OBJS)
	$(CC) -lreadline -lncurses -lc $(OBJS) -o $(TARGET)

# Directories for object files
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Rule to compile all object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(INDENT) -kr -i4 -nut -br $<
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Clean rule to remove object files and the target
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
