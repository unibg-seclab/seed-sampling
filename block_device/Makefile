SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:%.c=%.o)

# ------------ Compiler flags

CC = gcc
CFLAGS = -O3 -msse2 -msse -march=native -maes -Wno-cpp -Iinclude -Isrc
LDLIBS = -lblkio

# ------------ Generic building

build: $(OBJECTS)

# ------------ Build keygen utility

keygen: $(OBJECTS)

# ------------ Build benchmark

bench: $(OBJECTS)

# ------------ Cleaning

clean:
	@ rm -rf $(OBJECTS)
	@ rm -rf *.o
	@ rm -rf $(OUT) $(TEST) $(VERIFY) $(KEYMIXER)
	@ rm -rf $(LIBRARY)

clean_resources:
	@ rm -f $(RESOURCE) $(SECRET) $(ENC_RESOURCE)
