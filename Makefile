CC=g++ -std=c++17
CFLAGS=-Wall -I. -O2 -s -lsfml-graphics -lGL -lGLEW -pthread -lGLU
OBJDIR=obj
SRCDIR = ./src
SRC:=$(shell find $(SRCDIR) -name "*.cpp")
OBJ:=$(SRC:%.cpp=$(OBJDIR)/%.o)

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CC) -c -o $@  $< $(CFLAGS)

prog: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm $(shell find $(OBJDIR) -name "*.o")

run:
	./prog
