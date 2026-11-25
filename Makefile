CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = ocsh
SRCS = Main.c builtins.c pipes_signals.c
HDRS = builtins.h pipes_signals.h

all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o f.txt test.txt

.PHONY: all clean
