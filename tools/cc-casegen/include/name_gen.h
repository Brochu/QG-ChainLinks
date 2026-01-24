#pragma once
#include "qg_memory.hpp"
#include "shared_types.hpp"

#define NAME_GEN_MAX_NAMES 256
#define NAME_GEN_ALPHA_SIZE 58
#define NAME_GEN_BUF_LEN 64

struct name_gen_entry {
    i8 num_options = 0;
    char options[32];
    u8 counts[32];
};

struct name_gen {
    name_gen_entry *table = nullptr;
    const char *names[NAME_GEN_MAX_NAMES];
    u16 num_names = 0;
    mem_arena _mem;
};

// Train the Markov chain on a CSV file of names
void name_gen_train(name_gen *gen, const char *file_path);

// Free all memory
void name_gen_clear(name_gen *gen);

// Generate N base names (no prefix/suffix)
void name_gen_names(name_gen *gen, u32 num);

// Generate N district names with optional prefix/suffix
// e.g., "Silverton Heights", "Old Marbury", "Crestwood"
void name_gen_district(name_gen *gen, u32 num);

// Generate N street names with proper suffixes
// e.g., "Maple Street", "Oak Avenue", "River Boulevard"
void name_gen_street(name_gen *gen, u32 num);
