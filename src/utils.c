#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "types.h"
#include "utils.h"

size_t get_file_size(FILE *fp) {

        if (fp == NULL)
                return 0;

        if (fseek(fp, 0, SEEK_END) < 0)
                return 0;

        long res = ftell(fp);
        if (res < 0) {
                fprintf(stderr, "Cannot get size of file\n");
                return 0;
        }

        if (fseek(fp, 0, SEEK_SET) < 0)
                return 0;

        return (size_t)res;
}

byte *get_random_memory(size_t size) {
	byte *mem = malloc(size);
	int fd = open("/dev/urandom", O_RDONLY);

  if (size != read(fd, mem, size)){
  	free(mem);
  	mem = NULL;
	}

	close(fd);
	return mem;
}

void pprint_size(char *buf, size_t buf_size, size_t size){

	char *formatter;
	double hr_size = 0;
	if (size < 1024){
		formatter = "B";
		hr_size = size;
	} else if (size < 1024 * 1024) {
		formatter = "KiB";
		hr_size = (double)size / 1024;
	} else if (size < 1024 * 1024 * 1024){
		formatter = "MiB";
		hr_size = (double)size / (1024 * 1024);
	} else {
		formatter = "GiB";
		hr_size = (double)size / (1024 * 1024 * 1024);
	}
	snprintf(buf, buf_size, "%3.1f %s", hr_size, formatter);
}
