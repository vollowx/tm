CC=gcc
CFLAGS=-Wall -Wextra

TARGET=tm

SRC=main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /bin/$(TARGET)

unstall:
	rm -f /bin/$(TARGET)
