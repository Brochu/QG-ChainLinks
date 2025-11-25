#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "SDL3/SDL.h"
#include "qg_random.hpp"
#include "qg_generator.hpp"

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

    name_gen ctx;
    name_gen_train(&ctx, "../assets/city_names.csv");

    std::vector<std::string> out;
    name_gen_next(&ctx, 25, &out);
    for (int i = 0; i < out.size(); i++) {
        printf("[GEN] name: '%s'\n", out[i].c_str());
    }

    case_gen case_ctx {};
    case_gen_fondation(&case_ctx, city_size::SIZE_SMALL, 0);

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
    return 0;
}
