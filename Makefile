# Variables
CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lpanel -lncurses
TARGET = main
SRC = main.c

# Build Rule
all: $(TARGET)

debug: CFLAGS += -g -O0
debug: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(TARGET).c -o $(TARGET) $(LIBS) $(CFLAGS)
clean:
	rm -f $(TARGET)
