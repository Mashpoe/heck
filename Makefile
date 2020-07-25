CC = gcc
LIBS =
CFLAGS =

# recursive wildcard
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# compile everything in src
SRC=$(call rwildcard,src,*.c *.h)

TARGET=heck

.PHONY: all clean

all: $(TARGET)

clean:;rm -f $(TARGET)

$(TARGET): $(SRC);$(CC) -o $@ $^ $(CFLAGS) $(LIBS)