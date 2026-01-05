#include "qg_config.hpp"

#include "SDL3/SDL.h"
#include <cstdio>

void config_init(config *c, const char *file) {
    u64 file_size = 0;
    char *content = (char *)SDL_LoadFile(file, &file_size);

    char *next = NULL;
    char *token = strtok_s(content, "\r\n", &next);
    while (token) {
        printf("%s\n", token);
        token = strtok_s(NULL, "\r\n", &next);

        //TODO: Parse line by line
        // Split at '='
        // detect CSVs; split at ',' if the case
    }

    SDL_free(content);
}

void config_free(config *c) {
}
