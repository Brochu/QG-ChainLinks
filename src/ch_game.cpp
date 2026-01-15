#include "qg_config.hpp"
#include "qg_memory.hpp"
#include "qg_parse.hpp"
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
    config_value val;
    if (g_eng.config_read(&cfg, "ComposedWordLengthMax", &val)) {
        printf(" ComposedWordLengthMax -> '%d'\n", val.single);
    }
    if (g_eng.config_read(&cfg, "CrimeRandomRange", &val)) {
        printf(" CrimeRandomRange -> ['%d', '%d']\n", val.range.min, val.range.max);
    }
    if (g_eng.config_read(&cfg, "AssetsHomeDir", &val)) {
        printf(" AssetsHomeDir -> '%s'\n", val.str.arr);
    }

    u8 ws[2] = { 25, 12 };
    for (int i = 0; i < 10; i++) {
        i32 idx = rand_weighted_index(g_eng.rand_float01(), ws, 2);
        printf("RANDOM INDEX = %d\n", idx);
    }

    case_gen ctx;
    case_gen_init(&ctx);
    case_gen_fondation(&ctx, city_size::SIZE_MEDIUM, time(NULL));
    case_gen_population(&ctx);
    case_gen_clear(&ctx);

    strview text = sv("0,1,2,3,4,5,6,7,8,9");
    strview out[16];
    u64 n = g_eng.sv_split(text, ",", out, 16);
    for (int i = 0; i < n; i++) {
        printf("ELEM -> '" SV_FMT "'\n", SV_ARG(out[i]));
    }
}

void chain_tick(f32 dt) {
}

void chain_draw(f32 dt) {
}

void chain_exit() {
}
