#include "qg_config.hpp"
#include "qg_parse.hpp"

#include "SDL3/SDL.h"
#include <cstdio>

void config_init(config *c, const char *file) {
    u64 file_size = 0;
    char *content = (char *)SDL_LoadFile(file, &file_size);

    char *next = NULL;
    char *token = strtok_s(content, "\r\n", &next);
    while (token) {
        printf("%s\n", token);
        if (token[0] == '#') {
            // Comment line
            token = strtok_s(NULL, "\r\n", &next);
            continue;
        }

        strview l, r;
        bool res = sv_split_once(sv(token), " = ", &l, &r);
        if (res) {
            printf("\t -> key = '" SV_FMT "'; val = '" SV_FMT "'\n", SV_ARG(l), SV_ARG(r));
        }
        //TODO: Parse line by line
        // Split at '='
        // detect CSVs; split at ',' if the case

        token = strtok_s(NULL, "\r\n", &next);
    }

    SDL_free(content);
}

void config_free(config *c) {
}
