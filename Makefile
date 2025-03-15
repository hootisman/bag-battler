OBJS = main.cpp src/game.cpp src/render/graphics.cpp
OBJ_NAME = bag.out
CC = g++
COMPILER_FLAGS = -w
LINKER_FLAGS = -lSDL3 

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
