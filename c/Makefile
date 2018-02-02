# Environment
CC := gcc
CFLAGS := -Wall -O2 -std=c99
LDLIBS := -lm

ifneq ($(shell uname -s),Darwin)
	LDLIBS += -lrt
endif

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
