OBJS = main.cpp src/game.cpp src/util.cpp src/render/graphics.cpp src/render/camera.cpp src/render/shader.cpp src/model/model.cpp
OBJ_NAME = bag.out
CC = g++
COMPILER_FLAGS = -w -std=c++20 -Iinclude
LINKER_FLAGS = -lSDL3 -lassimp

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
