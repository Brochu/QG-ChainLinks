#include "qg_config.hpp"
#include "qg_memory.hpp"
#include "qg_random.hpp"
#include "shared.hpp"

#include "ch_generator.hpp"

#include <cstdio>
#include <ctime>

engine_api g_eng {};

void chain_init(engine_api engine) {
    g_eng = engine;

    mem_arena a;
    g_eng.mem_arena_init(&a, 1024);

    arena_off offset = g_eng.mem_arena_offloc(&a, 8, 8);
    i64 *value = mem_arena_at<i64>(&a, offset);
    *value = ~0;

    g_eng.mem_arena_clear(&a);

    for (int i = 0; i < 10; i++) {
        f32 val = g_eng.rand_float01();
        printf("%f\n", val);
    }

    config cfg;
    g_eng.config_init(&cfg, "../assets/city_gen.ini");
    printf("CONFIG VALUE = %d\n", cfg.value);

    u8 ws[2] = { 25, 12 };
    for (int i = 0; i < 10; i++) {
        i32 idx = rand_weighted_index(g_eng.rand_float01(), ws, 2);
        printf("RANDOM INDEX = %d\n", idx);
    }

    case_gen ctx;
    case_gen_init(&ctx);
    case_gen_fondation(&ctx, city_size::SIZE_SMALL, time(NULL));
    case_gen_population(&ctx);
    case_gen_clear(&ctx);
}

void chain_tick(f32 dt) {
}

void chain_draw(f32 dt) {
}

void chain_exit() {
}
