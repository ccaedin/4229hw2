# Makefile

CXX = g++
CXXFLAGS = -Wall -g -lSDL2 -lGL -lGLU -lSDL2_ttf

# Default target
all: compile

# Rule to compile
compile:
	$(CXX) -std=c++11 sdl.cpp -o output $(CXXFLAGS)

# Clean up
clean:
	rm -f output

.PHONY: all clean