SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:%.c=%.o)

SGEN = sgen

# ------------ Compiler flags

CC = gcc
CFLAGS = -march=native -Wno-cpp -Wall -Iinclude -I../include
LDLIBS = -lm -lcrypto 

$(SGEN): sgen.o $(OBJECTS)

entropy:
ifndef ENTROPY_PAGES
	$(error "Variable ENTROPY_PAGES is not defined")
endif
ifndef PAGE_SIZE
	$(error "Variable PAGE_SIZE is not defined")
endif
	@ echo [m] creating an entropy file with \($(ENTROPY_PAGES)\) pages of \($(PAGE_SIZE)\) bytes...
	dd status=progress if=/dev/zero of=$(FNAME) bs=$(PAGE_SIZE) count=$(ENTROPY_PAGES)

.PHONY: run
run: $(SGEN)
	@ ./$(SGEN) "$(FNAME)" $(PAGE_SIZE) $(SEED_PAGES) $(ENTROPY_PAGES) $(REPS) $(KEY) $(IV)

.PHONY: sizes
sizes:
	@ echo "4 KiB = 1 4-KiB pages"
	@ echo "1 MiB = 256 4-KiB pages"
	@ echo "1 GiB = 262144 4-KiB pages"
	@ echo "10 GiB = 2621440 4-KiB pages"
	@ echo "50 GiB = 13107200 4-KiB pages"
	@ echo "200 GiB = 52428800 4-KiB pages"

clean:
	@ $(foreach obj,$(OBJECTS), $(shell rm -rf $(obj)))
	@ rm -rf $(SGEN) $(SGEN).o
	@ rm -rf entropy
