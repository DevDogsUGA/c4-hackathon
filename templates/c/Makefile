CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -O2
LDFLAGS ?=
LDLIBS = -lm

# Every .c file in this folder is compiled in, so you can add your own files.
SOURCES = $(wildcard *.c) vendor/cJSON.c
TARGET = bot

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(wildcard *.h) vendor/cJSON.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o vendor/*.o
