#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <windows.h>

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_render.h"

#include "qg_config.hpp"
#include "qg_generator.hpp"
#include "qg_random.hpp"

// ------------- GAMELIB LOADING

#define GAMELIB_BASE_PATH "./gamelibs/chain.dll"
#define GAMELIB_LOAD_PATH "./gamelibs/chain_loaded.dll"

#define GAMELIB_FUNCS \
    X(void, game_init, "chain_init", (void)) \
    X(void, game_tick, "chain_tick", (float)) \
    X(void, game_draw, "chain_draw", (float)) \
    X(void, game_close, "chain_exit", (void))

#define X(ret, name, sym, args) typedef ret (*name##_t) args;
GAMELIB_FUNCS
#undef X

#define X(ret, name, sym, args) name##_t name = NULL;
GAMELIB_FUNCS
#undef X

HMODULE gamelib_mod = NULL;

void gamelib_load() {
    assert(gamelib_mod == NULL && "Did not properly free the gamelib module");

    if (GetFileAttributes(GAMELIB_LOAD_PATH) == INVALID_FILE_ATTRIBUTES) {
        CopyFile(GAMELIB_BASE_PATH, GAMELIB_LOAD_PATH, false);
    }
    gamelib_mod = LoadLibrary(GAMELIB_LOAD_PATH);
    assert(gamelib_mod != NULL && "Failed to load game library");

    #define X(ret, name, sym, args) \
        name = (name##_t)GetProcAddress(gamelib_mod, sym); \
        assert(name != NULL && "Failed to load function " sym);
    GAMELIB_FUNCS
    #undef X
}

void gamelib_free() {
    if (gamelib_mod != NULL) {
        FreeLibrary(gamelib_mod);
        gamelib_mod = NULL;
    }
}

void gamelib_refresh() {
    gamelib_free();
    CopyFile(GAMELIB_BASE_PATH, GAMELIB_LOAD_PATH, false);
    gamelib_load();
}

// ------------- ENGINE - MAIN

#define VERSION "ALPHA" //TODO: Export version from the main game DLL
#define TITLE "QG-ChainLinks"
#define WIDTH 800
#define HEIGHT 600

bool qg_running = true;
float pos = 0.f;

config root_cfg;

int main(int argc, char **argv) {
    rand_seed(time(NULL));
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        printf("Could not init SDL3\nerror: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    printf("[QG] SDL3 Correctly init'ed!\n");
    //gamelib_load();

    game_init();

    case_gen ctx = {};
    case_gen_init(&ctx);
    case_gen_fondation(&ctx, city_size::SIZE_METRO, rand_int(INT_MAX));
    case_gen_population(&ctx);
    case_gen_clear(&ctx);

    //gamelib_free();
    SDL_Quit();
    printf("[QG] quitting SDL3!\n");
    return 0;

    SDL_Window *window;
    SDL_Renderer *context;
    if (!SDL_CreateWindowAndRenderer("[" VERSION "]" TITLE, 800, 600, 0, &window, &context)) {
        printf("Could not create Window or Renderer\nerror: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Event event;
    while (qg_running) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                qg_running = false;
            }
            if (event.type == SDL_EVENT_KEY_UP && event.key.key == SDLK_ESCAPE) {
                qg_running = false;
            }
        }

        // Update State
        pos += 0.01f;

        // Draw current frame
        SDL_SetRenderDrawColor(context, 255, 255, 255, 255);
        SDL_RenderFillRect(context, NULL);

        SDL_SetRenderDrawColor(context, 255, 0, 0, 255);
        SDL_FRect r;
        r.x = r.y = pos;
        r.w = r.h = 100.f;
        SDL_RenderFillRect(context, &r);

        SDL_RenderPresent(context);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("[QG] quitting SDL3!\n");
    return 0;
}
