#include "qg_memory.hpp"

void mem_arena_init(mem_arena *arena, u64 max_size) {
    arena->gen = 0;
    arena->next = 0;

    arena->base = nullptr;
    arena->cap = max_size;
}

arena_ptr mem_arena_alloc(mem_arena *arena, u64 size) {
    return {};
}

void mem_arena_reset(mem_arena *arena) {
    arena->next = 0;
    arena->gen++;
}

void mem_arena_clear(mem_arena *arena) {
}
