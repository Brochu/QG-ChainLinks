#include <stdio.h>
#include <stdlib.h>

#include "SDL3/SDL.h"

int main(int argc, char **argv) {
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        printf("Could not init SDL3\nerror: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    printf("[ALIBI] My own starting point!\n");

    SDL_Quit();
    return 0;
}
