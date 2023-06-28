TARGET = pokecollection
CC = gcc
CFLAGS = -g -std=c11 -Wall
LFLAGS = -lsqlite3 -lcurl -lcjson

HEADERS = commands.h db.h data_retrieval.h linked_list.h
OBJECTS = main.o commands.o db.o data_retrieval.o linked_list.o

default: $(TARGET)
all: default

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)