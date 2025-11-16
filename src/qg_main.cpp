#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "SDL3/SDL.h"
#include "sqlite3.h"
#include "generator.hpp"

#define VERSION "ALPHA" //TODO: Export version from the main game DLL
#define TITLE "QG-ChainLinks"
#define WIDTH 800
#define HEIGHT 600

bool qg_running = true;
float pos = 0.f;

int debug_callback(void *data, int size, char **var0, char **var1) {
    return 0;
}

int main(int argc, char **argv) {
    srand(time(NULL));
    rand();

    for (int i = 0; i < 5; i++) {
        int idx = rand_weighted_index(district_weights, DISTRICT_COUNT);
        printf("[GEN] Random weighted ID = %i\n", idx);
    }

    name_generator gen;
    name_generator_train(&gen);

    std::vector<std::string> names;
    name_generator_next(&gen, 15, &names);
    for (int i = 0; i < names.size(); i++) {
        printf("[GEN] name = '%s'\n", names[i].c_str());
    }

    sqlite3 *db;
    int res = sqlite3_open("./detective.db", &db);
    printf("[PROC-GEN] Opening Case DB; res = %i\n", res);

    char *err = nullptr;
    res = sqlite3_exec(db, "SELECT * FROM users;", debug_callback, NULL, &err);
    printf("[PROC-GEN] EXEC res = %i; err = %s\n", res, err);

    res = sqlite3_close_v2(db);
    printf("[PROC-GEN] Closing Case DB; res = %i\n", res);
    printf("[PROC-GEN] Just testing, SQLITE_OK value = %i\n", SQLITE_OK);

    return 0;
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        printf("Could not init SDL3\nerror: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    printf("[QG] opening SDL3 window!\n");

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
    printf("[QG] quitting SDL3 window!\n");
    return 0;
}
