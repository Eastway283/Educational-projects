CC = gcc
CFLAGS = -Iinclude -Wall -Wextra
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/button.c $(SRCDIR)/array.c $(SRCDIR)/algorithms.c $(SRCDIR)/structures.c
TARGET = visalgo

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
