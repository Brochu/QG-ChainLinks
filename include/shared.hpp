#pragma once
#include "shared_types.hpp"

// ENGINE ======================================

struct qg_config_api;
struct qg_memory_api;
struct qg_random_api;

struct engine_api {
    qg_config_api *conf;
    qg_memory_api *mem;
    qg_random_api *rand;
};

// GAME   ======================================

#define CHAIN_API __declspec(dllexport)

extern "C" void CHAIN_API chain_init(engine_api engine);
extern "C" void CHAIN_API chain_tick(f32 dt);
extern "C" void CHAIN_API chain_draw(f32 dt);
extern "C" void CHAIN_API chain_exit();

#define GAME_MODULE_DEF \
    X(void, game_init, "chain_init", (engine_api)) \
    X(void, game_tick, "chain_tick", (float)) \
    X(void, game_draw, "chain_draw", (float)) \
    X(void, game_exit, "chain_exit", (void))
