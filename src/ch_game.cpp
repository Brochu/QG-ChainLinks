#include "shared.hpp"

#include "qg_config.hpp"
#include "qg_memory.hpp"
#include "qg_random.hpp"

#include <cstdio>

void chain_init(engine_api engine) {
    mem_arena a;
    engine.mem->mem_arena_init(&a, 1024);

    arena_off offset = engine.mem->mem_arena_offloc(&a, 8, 8);
    i64 *value = mem_arena_at<i64>(&a, offset);
    *value = ~0;

    engine.mem->mem_arena_clear(&a);

    for (int i = 0; i < 10; i++) {
        f32 val = engine.rand->rand_float01();
        printf("%f\n", val);
    }

    config cfg;
    engine.conf->config_init(&cfg, "");
    printf("CONFIG VALUE = %d\n", cfg.value);

    u8 ws[2] = { 25, 12 };
    i32 idx = rand_weighted_index(engine.rand->rand_float01(), ws, 2);
    printf("RANDOM INDEX = %d\n", idx);
}

void chain_tick(f32 dt) {
}

void chain_draw(f32 dt) {
}

void chain_exit() {
}
