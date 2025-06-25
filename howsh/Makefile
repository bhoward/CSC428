TARGET = howsh
CC = gcc
CFLAGS = -Wall

HEADERS = command.h pipeline.h util.h parse.h
OBJECTS = command.o pipeline.o util.o parse.o main.o

default: $(TARGET)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
