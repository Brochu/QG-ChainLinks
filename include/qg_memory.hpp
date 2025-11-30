#pragma once

#include "qg_types.hpp"

void *qg_malloc(u64 sz);
void *qg_calloc(u64 count, u64 sz);
void *qg_realloc(void *ptr, u64 sz);
void qg_free(void *ptr);

struct mem_arena {
    u8 *base;
    u64 next;
    u64 cap;
    u64 gen;
};

struct arena_ptr {
    u8 *p;
    u64 gen;
};

void mem_arena_init(mem_arena *arena, u64 max_size);
arena_ptr mem_arena_alloc(mem_arena *arena, u64 size, u64 align = sizeof(void *));
void mem_arena_reset(mem_arena *arena);
void mem_arena_clear(mem_arena *arena);
