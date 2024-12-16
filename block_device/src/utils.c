#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int get_num_tokens(char *arg, const char *delimiter) {
    int len = strlen(arg);
    char argcpy[len + 1];
    char *token;
    int num_tokens;

    strcpy(argcpy, arg);
    num_tokens = 0;

    token = strtok(argcpy, delimiter);
    while (token) {
        num_tokens++;
        token = strtok(NULL, delimiter);
    };

    return num_tokens;
}

uint8_t get_hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0xFF;
}

int parse_hex(char *hex, unsigned char *buf, size_t size) {
    int res;
    uint8_t msn;
    uint8_t lsn;

    for (res = 0; *hex; res++) {
        msn = get_hex_value(*hex++);
        if (msn == 0xFF) return -1;
        lsn = get_hex_value(*hex++);
        if (msn == 0xFF) return -1;

        if (size-- <= 0) return -1;
        *buf++ = (msn << 4) + lsn;
    }

    return res;
}
