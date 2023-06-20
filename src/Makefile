TARGET = main
CC = gcc
CFLAGS = -g -std=c11 -Wall

HEADERS = commands.h
OBJECTS = main.o commands.o

default: $(TARGET)
all: default

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)