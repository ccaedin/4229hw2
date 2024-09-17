# Makefile

CXX = g++
CXXFLAGS = -Wall -g -lSDL2 -lGL -lGLU -lSDL2_ttf
# Default target
all: compile

# Rule to compile
compile:
	$(CXX) -std=c++11 lorenz.cpp Text.cpp -o hw2 $(CXXFLAGS)

# Clean up
clean:
	rm -f output

.PHONY: all clean