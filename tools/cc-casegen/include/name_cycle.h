#pragma once
#include "qg_memory.hpp"
#include "shared_types.hpp"

#define MAX_NUM_NAMES 1024

struct name_cycle {
    const char *names[MAX_NUM_NAMES];
    i16 num_names = 0;
    u64 step;
    u64 next;
    mem_arena _mem;
};

// Initialize name cycle from a CSV file (comma-separated, single line)
// Uses prime step cycling to iterate through names without immediate repeats
void name_cycle_init(name_cycle *ctx, const char *file_path);

// Free memory used by name cycle
void name_cycle_clear(name_cycle *ctx);

// Get next name from cycle (returns pointer to internal storage)
const char *name_cycle_next(name_cycle *ctx);
