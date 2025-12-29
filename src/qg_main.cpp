#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_render.h"

#include "qg_generator.hpp"
#include "qg_random.hpp"

#define VERSION "ALPHA" //TODO: Export version from the main game DLL
#define TITLE "QG-ChainLinks"
#define WIDTH 800
#define HEIGHT 600

bool qg_running = true;
float pos = 0.f;

int main(int argc, char **argv) {
    rand_seed(time(NULL));
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        printf("Could not init SDL3\nerror: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    printf("[QG] SDL3 Correctly init'ed!\n");

    case_gen ctx = {};
    case_gen_init(&ctx);
    case_gen_fondation(&ctx, city_size::SIZE_SMALL, rand_int(INT_MAX));
    case_gen_clear(&ctx);

    /*
    name_gen district_gen;
    name_gen_train(&district_gen, "../assets/city_names.csv");

    name_gen_next(&district_gen, 10);
    for (i32 i = 0; i < district_gen.num_names; i++) {
        printf(" - '%s'\n", district_gen.names[i]);
    }
    printf("-=-=-=-= \n");

    name_gen_district(&district_gen, 10);
    for (i32 i = 0; i < district_gen.num_names; i++) {
        printf(" - '%s'\n", district_gen.names[i]);
    }
    printf("-=-=-=-= \n");
    name_gen_clear(&district_gen);

    name_cycle char_gen;
    name_cycle_init(&char_gen, "../assets/f_names.csv");
    for (i32 i = 0; i < 10; i++) {
        const char *name = name_cycle_next(&char_gen);
        printf(" - '%s'\n", name);
    }
    printf("-=-=-=-= \n");
    name_cycle_clear(&char_gen);
    */

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
