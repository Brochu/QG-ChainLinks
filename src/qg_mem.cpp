#include "qg_memory.hpp"
#include <cstdlib>

//TODO: Add memory tracking w/ profiler? Tracy
void *qg_malloc(u64 size) {
    return malloc(size);
}

void *qg_calloc(u64 count, u64 size) {
    return calloc(count, size);
}

void *qg_realloc(void *ptr, u64 size) {
    return realloc(ptr, size);
}

void qg_free(void *ptr) {
    free(ptr);
}

// MEMORY ARENA -----------------------------------

void mem_arena_init(mem_arena *arena, u64 max_size) {
    arena->gen = 0;
    arena->next = 0;

    //TODO: Handle alloc
    arena->base = nullptr;
    arena->cap = max_size;
}

arena_ptr mem_arena_alloc(mem_arena *arena, u64 size) {
    //TODO: Fetch next size bytes create handle to it
    return { nullptr, arena->gen };
}

void mem_arena_reset(mem_arena *arena) {
    arena->next = 0;
    arena->gen++;
}

void mem_arena_clear(mem_arena *arena) {
    //TODO: Handle de-alloc
}
