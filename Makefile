srctree = .
IDIR = include

CC = gcc
LIBS =
CFLAGS = -I$(srctree)/$(IDIR)

BDIR = build
LDIR = lib
SDIR = src

TARGET = heck
DEPS = $(shell find include/ -type f -name '*.h') $(shell find src/ -type f -name '*.h')
SRC = $(shell find src/ -type f -name '*.c')
OBJ = $(patsubst $(SDIR)/%.c, $(BDIR)/%.o, $(SRC))

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f $(TARGET)
	rm -r $(BDIR)

$(BDIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p "$(@D)"
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)