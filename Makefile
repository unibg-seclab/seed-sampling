SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:%.c=%.o)

SAMPLING = sampling

# ------------ Compiler flags

CC = gcc
CFLAGS = -march=native -Wno-cpp -Wall -Iinclude -I../include
LDLIBS = -lm

$(SAMPLING): sampling.o $(OBJECTS)

entropy:
ifndef ENTROPY_PAGES
	$(error "Variable ENTROPY_PAGES is not defined")
endif
ifndef PAGE_SIZE
	$(error "Variable PAGE_SIZE is not defined")
endif
	@ echo [m] creating an entropy file with \($(ENTROPY_PAGES)\) pages of \($(PAGE_SIZE)\) bytes...
	dd if=/dev/zero of=$(FNAME) bs=$(PAGE_SIZE) count=$(ENTROPY_PAGES) 2>/dev/null

main: main.o $(OBJECTS)

.PHONY: run
run: entropy $(SAMPLING)
	@ ./$(SAMPLING) "$(FNAME)" $(PAGE_SIZE) $(SEED_PAGES) $(ENTROPY_PAGES) $(REPS)

.PHONY: sizes
sizes:
	@ echo "4 KiB = 1 4-KiB pages"
	@ echo "1 MiB = 256 4-KiB pages"
	@ echo "1 GiB = 262144 4-KiB pages"
	@ echo "10 GiB = 2621440 4-KiB pages"
	@ echo "50 GiB = 13107200 4-KiB pages"

clean:
	@ $(foreach obj,$(OBJECTS), $(shell rm -rf $(obj)))
	@ rm -rf $(SAMPLING) $(SAMPLING).o
	@ rm -rf entropy
