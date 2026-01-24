#include "name_cycle.h"
#include "qg_memory.hpp"
#include "qg_random.hpp"

#include "SDL3/SDL.h"
#include <cstring>

#define NAME_ARENA_SIZE (MAX_NUM_NAMES * 20)

void name_cycle_init(name_cycle *ctx, const char *file_path) {
    if (ctx->_mem.base == nullptr) {
        mem_arena_init(&ctx->_mem, NAME_ARENA_SIZE);
    } else {
        mem_arena_reset(&ctx->_mem);
    }
    ctx->num_names = 0;

    u64 file_size = 0;
    char *file_contents = (char*)SDL_LoadFile(file_path, &file_size);
    if (!file_contents) {
        return;
    }

    char *s = file_contents;
    char *e = strstr(file_contents, ",");

    while (e != NULL && ctx->num_names < MAX_NUM_NAMES) {
        const u64 len = (e - s) + 1;
        arena_ptr pname = mem_arena_alloc(&ctx->_mem, len, 1);

        memcpy(pname.p, s, len - 1);
        pname.p[len - 1] = '\0';
        ctx->names[ctx->num_names++] = (const char *)pname.p;

        s = e + 1;
        e = strstr(s, ",");
    }

    // Handle last name (no trailing comma)
    if (*s && ctx->num_names < MAX_NUM_NAMES) {
        u64 len = strlen(s);
        // Trim newline if present
        while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
            len--;
        }
        if (len > 0) {
            arena_ptr pname = mem_arena_alloc(&ctx->_mem, len + 1, 1);
            memcpy(pname.p, s, len);
            pname.p[len] = '\0';
            ctx->names[ctx->num_names++] = (const char *)pname.p;
        }
    }

    SDL_free((void*)file_contents);

    // Use prime steps for cycling to avoid immediate repeats
    static u32 prime_steps[] = { 2, 29, 73, 113, 179, 229, 283, 349, 419, 463, 547, 601 };
    static u64 num_prime_steps = sizeof(prime_steps) / sizeof(prime_steps[0]);

    ctx->step = prime_steps[rand_int(num_prime_steps)];
    // Ensure step and num_names are coprime for full coverage
    while (ctx->num_names > 0 && ctx->step % ctx->num_names == 0) {
        ctx->step++;
    }
    ctx->next = (ctx->num_names > 0) ? rand_int(ctx->num_names) : 0;
}

void name_cycle_clear(name_cycle *ctx) {
    mem_arena_clear(&ctx->_mem);
    ctx->num_names = 0;
    ctx->step = 0;
    ctx->next = 0;
}

const char *name_cycle_next(name_cycle *ctx) {
    if (!ctx || ctx->num_names <= 0) {
        return nullptr;
    }

    const char *selected = ctx->names[ctx->next];
    ctx->next = (ctx->next + ctx->step) % ctx->num_names;

    return selected;
}
