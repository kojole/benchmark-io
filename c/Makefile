# Environment
CC := gcc
CFLAGS := -Wall -O2 -std=c11
LDLIBS = -lm

# Project
program := main
sources := $(wildcard *.c)
objects := $(sources:.c=.o)

$(program): $(objects)

# Targets
.DEFAULT_GOAL := all
.PHONY: all build clean format

all: build

build: $(program)

clean:
	-$(RM) *.o $(program)

format:
	clang-format -i *.c *.h
