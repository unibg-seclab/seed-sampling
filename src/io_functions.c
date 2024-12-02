#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "io_functions.h"
#include "types.h"
#include "utils.h"

int single_fseek(FILE *f) {

	long offset = 0;
	if (-1 == fseek(f, offset, SEEK_SET)){
		printf("single_fseed: fseek failed\n");				
		return -1;
	}

	return 0;
}

int single_fread(FILE *f, byte *buf, size_t page_size){

	if (1 != fread(buf, page_size, 1, f)){
		printf("single_fread: cannot read enough memory\n");
		return -1;
	}

	return 0;
}


int scan(FILE *f, byte *seed, size_t page_size, size_t pages){
	// fseek
	long offset = 0;
	offset = fseek(f, offset, SEEK_SET);
	if (offset == -1){
		printf("scan: fseek failed\n");		
		return -1;
	}
	// fread
	if (pages != fread(seed, page_size, pages, f)){
		printf("scan: cannot read enough memory\n");
		return -1;
	}

	return 0;
}

// Reads the i-th 4-byte index from the pool of indexes
static inline unsigned long idx_to_pos(byte *indexes, const unsigned int i, const unsigned int tot_pages) {

	unsigned int b = i * 4;
	unsigned int pos = indexes[b] << 24;
	pos += indexes[b+1] << 16;
	pos += indexes[b+2] << 8;
	pos += indexes[b+3];
	
	return pos % tot_pages;
}

int random_read(FILE *f, byte *seed, byte *indexes, size_t page_size, size_t pages){

	const unsigned int tot_entropy_pages = get_file_size(f) / page_size;

	unsigned long offset;
	for (size_t i = 0; i < pages; i++){
		// get the i-th offset
		offset = idx_to_pos(indexes, i, tot_entropy_pages);
		// fseek
		if (-1 == fseek(f, offset, SEEK_SET)){
			printf("random_read: fseek failed\n");			
			return -1;
		}
		// fread
		if (1 != fread(seed + i * page_size, page_size, 1, f)){
			printf("random_read: cannot read enough memory\n");
			return -1;
		}
	}

	return 0;
}

