# Environment
CC := gcc
override CFLAGS += -Wall -O2 -std=c99
LDLIBS := -lm

ifeq ($(shell uname -s),Darwin)
	override CFLAGS += -DUSE_FSYNC
else
	override CFLAGS += -pthread
	LDLIBS += -lrt -pthread
endif

# Project
program := benchmark-io-c
sources := $(wildcard *.c)
objects := $(sources:.c=.o)

$(program): $(objects)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) $(OUTPUT_OPTION)

# Targets
.DEFAULT_GOAL := all
.PHONY: all build clean format

all: build

build: $(program)

clean:
	-$(RM) *.o $(program)

format:
	clang-format -i *.c *.h
