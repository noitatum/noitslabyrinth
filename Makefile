# No default suffixes
.SUFFIXES:
# These targets are not files
.PHONY: all windows clean debug
# Compiler
CC = g++
# Compiler flags
CFLAGS = -Wall -Wextra -Wno-unused-result -Isrc/include/
# Optimization flags
OPT_FLAGS = -O2 -flto
# Linker flags
LFLAGS = -lGL -lm -ldl -lglfw
# Executable
EXECUTABLE = bin/maze
# Build Directories
DIRECTORIES = bin/ obj/
# Source directory
SRC_DIR = src/
# Object directory
OBJ_DIR = obj/
# Object files
OBJS = main.o stb_image.o gl3w.o maze.o shader.o
# Object full path
OBJS_FULL = $(addprefix $(OBJ_DIR), $(OBJS))

all: $(DIRECTORIES) $(EXECUTABLE)

clean:
	rm -rf $(DIRECTORIES)

debug: OPT_FLAGS=-ggdb
debug: all

$(DIRECTORIES):
	mkdir -p $@

$(EXECUTABLE): $(OBJS_FULL)
	$(CC) $^ $(LFLAGS) $(OPT_FLAGS) -o $(EXECUTABLE)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $< $(CFLAGS) $(OPT_FLAGS) -c -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	$(CC) $< $(CFLAGS) $(OPT_FLAGS) -c -o $@
