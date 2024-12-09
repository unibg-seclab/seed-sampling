#include <ctype.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
        int fd    = open("/dev/urandom", O_RDONLY);

        if (size != read(fd, mem, size)) {
                free(mem);
                mem = NULL;
        }
        close(fd);

        return mem;
}

void pprint_size(char *buf, size_t buf_size, size_t size) {

        char *formatter;
        double hr_size = 0;
        if (size < 1024) {
                formatter = "B";
                hr_size   = size;
        } else if (size < 1024 * 1024) {
                formatter = "KiB";
                hr_size   = (double)size / 1024;
        } else if (size < 1024 * 1024 * 1024) {
                formatter = "MiB";
                hr_size   = (double)size / (1024 * 1024);
        } else {
                formatter = "GiB";
                hr_size   = (double)size / (1024 * 1024 * 1024);
        }
        snprintf(buf, buf_size, "%3.1f %s", hr_size, formatter);
}

int is_valid_char(char *hex) {

        byte b = tolower(*hex);
        if ((b >= '0' && b <= '9') || (b >= 'a' && b <= 'f'))
                return 1;

        return 0;
}

byte from_char_pair_to_byte(char *sx, char *dx) {

        byte _sx, _dx;
        _sx = tolower(*sx);
        _dx = tolower(*dx);

        if (_sx >= '0' && _sx <= '9')
                _sx = _sx - '0';
        else if (_sx >= 'a' && _sx <= 'f')
                _sx = _sx - 'a' + 10;

        if (_dx >= '0' && _dx <= '9')
                _dx = _dx - '0';
        else if (_dx >= 'a' && _dx <= 'f')
                _dx = _dx - 'a' + 10;

        return (_sx << 4) + _dx;
}

int parse_hex(byte *secret, uint8_t secret_size, char *hex) {

        for (uint8_t i = 0; i < secret_size; i++) {
                if (!is_valid_char(hex + 2 * i) || !is_valid_char(hex + 2 * i + 1))
                        return -1;
                secret[i] = from_char_pair_to_byte(hex + 2 * i, hex + 2 * i + 1);
        }

        return 0;
}
