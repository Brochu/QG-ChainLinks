#pragma once
#include "shared_types.hpp"

// ENGINE ======================================

enum class value_type : u8;
struct config_value;
struct config;
#define CONFIG_MODULE_DEF \
    X(void, config_init, (config*, const char*)) \
    X(void, config_free, (config*)) \
    X(bool, config_read, (config*, const char*, config_value*))

struct mem_arena;
struct arena_ptr;
struct arena_off;
#define MEMORY_MODULE_DEF \
    X(void*, qg_malloc, (u64)) \
    X(void*, qg_calloc, (u64, u64)) \
    X(void*, qg_realloc, (void*, u64)) \
    X(void, qg_free, (void*)) \
    X(void, mem_arena_init, (mem_arena*, u64)) \
    X(void, mem_arena_reset, (mem_arena*)) \
    X(void, mem_arena_clear, (mem_arena*)) \
    X(arena_ptr, mem_arena_alloc, (mem_arena*, u64, u64)) \
    X(arena_off, mem_arena_offloc, (mem_arena*, u64, u64))

struct strview;
#define PARSE_MODULE_DEF \
    X(strview, sv_find, (strview, const char*)) \
    X(u64, sv_split, (strview, const char*, strview*, u64)) \
    X(bool, sv_split_once, (strview, const char*, strview*, strview*)) \

#define RANDOM_MODULE_DEF \
    X(void, rand_seed, (i64)) \
    X(f32, rand_float01, (void)) \
    X(i32, rand_int, (i32)) \
    X(i32, rand_int_min, (i32, i32))  \
    X(i8, rand_actor_age, (void))

struct engine_api {
    #define X(ret, name, params) ret (*name) params;

    struct { CONFIG_MODULE_DEF };
    struct { MEMORY_MODULE_DEF };
    struct { PARSE_MODULE_DEF };
    struct { RANDOM_MODULE_DEF };

    #undef X
};

extern engine_api g_eng;

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
