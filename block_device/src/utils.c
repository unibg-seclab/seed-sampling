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
