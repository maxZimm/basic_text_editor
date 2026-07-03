# Variables
CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lpanel -lncurses
TARGET = main

# Build Rule
all:
	$(CC) $(TARGET).c -o $(TARGET) $(LIBS) $(CFLAGS)

clean:
	rm -f $(TARGET)
