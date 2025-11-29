#pragma once

struct mem_arena {
    unsigned char *base;
    size_t next;
    size_t cap;
    size_t gen;
};

struct arena_ptr {
    unsigned char *p;
    size_t gen;
};

void mem_arena_init(mem_arena *arena, size_t max_size);
arena_ptr mem_arena_alloc(mem_arena *arena, size_t size);
void mem_arena_reset(mem_arena *arena);
void mem_arena_clear(mem_arena *arena);
