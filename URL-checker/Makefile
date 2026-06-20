# Компилятор и флаги
CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra
LDFLAGS = -pthread

# Имя исполняемого файла
TARGET = checker

# Исходные файлы
SRCS = main.c url.c 
OBJS = $(SRCS:.c =.o)

# Правило по умолчанию (просто make)
all: $(TARGET)

# Линковка исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Компиляция каждого .c в .o
%.o: %.c url.h
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка скомпилированных файлов
clean:
	rm -f $(TARGET) $(OBJS)

# Указание, что цели clean и all не являются файлами
.PHONY: all clean
