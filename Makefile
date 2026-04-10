CC = clang
CFLAGS = -Wall -Wextra -O2
TARGET = preallocate
SRC = preallocate.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
