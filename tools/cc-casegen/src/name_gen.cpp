#include "name_gen.h"
#include "qg_memory.hpp"
#include "qg_random.hpp"

#include <SDL3/SDL.h>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>

// ============================================================================
// ASCII TO DENSE INDEX MAPPING
// ============================================================================

#define GEN_MAP_SIZE 256

constexpr std::array<u8, GEN_MAP_SIZE> make_ascii_to_dense() {
    std::array<u8, GEN_MAP_SIZE> arr {};
    for (int i = 0; i < GEN_MAP_SIZE; i++) {
        arr[i] = 0xFF;
    }
    for (int c = 'A'; c <= 'Z'; c++) {
        arr[c] = c - 'A';
    }
    for (int c = 'a'; c <= 'z'; c++) {
        arr[c] = 26 + (c - 'a');
    }
    arr['^'] = 52;  // Start marker
    arr['$'] = 53;  // End marker
    arr[' '] = 54;
    arr['.'] = 55;
    arr['-'] = 56;
    arr['\''] = 57;
    return arr;
}

static constexpr std::array<u8, GEN_MAP_SIZE> ascii_to_dense = make_ascii_to_dense();

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

static bool name_gen_validate(const char *name, i32 len) {
    i32 f_space = INT32_MAX;
    i32 l_space = INT32_MIN;
    for (i32 i = 0; i < len; i++) {
        if (name[i] == ' ') {
            f_space = (f_space < i) ? f_space : i;
            l_space = (l_space > i) ? l_space : i;
        }
    }

    // Max one space allowed
    if ((f_space != INT32_MAX || l_space != INT32_MIN) && f_space != l_space) return false;

    // Single word: 4-12 chars
    if (f_space == INT32_MAX && (len > 12 || len < 4)) return false;

    // Two words: each part max 10 chars
    if (f_space != INT32_MAX && (f_space > 10 || len - f_space - 1 > 10)) return false;

    return true;
}

static i32 name_gen_create(name_gen *gen, char *name) {
    const i32 k = 3;
    name[0] = name[1] = name[2] = '^';

    char next = '\0';
    i32 i;
    for (i = 0; next != '$' && i < NAME_GEN_BUF_LEN - 4; i++) {
        i32 hi = ascii_to_dense[(u8)name[i+0]] * NAME_GEN_ALPHA_SIZE * NAME_GEN_ALPHA_SIZE;
        i32 mid = ascii_to_dense[(u8)name[i+1]] * NAME_GEN_ALPHA_SIZE;
        i32 lo = ascii_to_dense[(u8)name[i+2]];
        i32 idx = hi + mid + lo;

        if (idx >= NAME_GEN_ALPHA_SIZE * NAME_GEN_ALPHA_SIZE * NAME_GEN_ALPHA_SIZE) {
            return -1;  // Invalid state
        }

        name_gen_entry *e = &gen->table[idx];
        if (e->num_options == 0) {
            return -1;  // Dead end
        }

        int next_idx = rand_weighted_index(rand_float01(), e->counts, e->num_options);
        next = e->options[next_idx];
        name[i+3] = next;
    }

    // Remove ^^^ prefix and $ suffix
    memmove(name, name + 3, i);
    name[i - 1] = '\0';

    return i - 1;
}

static const char *alloc_name(mem_arena *arena, const char *name, i32 len) {
    char *p = (char *)mem_arena_alloc(arena, len + 1, 1).p;
    memcpy(p, name, len);
    p[len] = '\0';
    return p;
}

// ============================================================================
// PUBLIC API
// ============================================================================

void name_gen_train(name_gen *gen, const char *file_path) {
    // Allocate memory for trigram table + generated names
    u64 table_size = NAME_GEN_ALPHA_SIZE * NAME_GEN_ALPHA_SIZE * NAME_GEN_ALPHA_SIZE * sizeof(name_gen_entry);
    u64 names_size = NAME_GEN_MAX_NAMES * 32;

    if (gen->_mem.base == nullptr) {
        mem_arena_init(&gen->_mem, table_size + names_size);
    } else {
        mem_arena_reset(&gen->_mem);
    }

    gen->table = (name_gen_entry *)mem_arena_alloc(&gen->_mem, table_size, sizeof(void *)).p;
    memset(gen->table, 0, table_size);
    gen->num_names = 0;

    // Load CSV file
    u64 file_len = 0;
    char *file_contents = (char *)SDL_LoadFile(file_path, &file_len);
    if (!file_contents) {
        printf("[NAME_GEN] Could not load %s\n", file_path);
        return;
    }

    // Parse CSV and build trigram table
    char *s = file_contents;
    char *e = strstr(s, ",");
    const i32 k = 3;

    while (e != NULL) {
        char buffer[NAME_GEN_BUF_LEN] = "^^^";
        i32 name_len = (i32)(e - s);
        if (name_len > 0 && name_len < NAME_GEN_BUF_LEN - 7) {
            strncat(buffer, s, name_len);
            strcat(buffer, "$$$");
            i32 buf_len = name_len + 6;

            for (i32 i = 0; i < buf_len - k; i++) {
                i32 hi = ascii_to_dense[(u8)buffer[i+0]] * NAME_GEN_ALPHA_SIZE * NAME_GEN_ALPHA_SIZE;
                i32 mid = ascii_to_dense[(u8)buffer[i+1]] * NAME_GEN_ALPHA_SIZE;
                i32 lo = ascii_to_dense[(u8)buffer[i+2]];
                i32 idx = hi + mid + lo;

                if (idx >= NAME_GEN_ALPHA_SIZE * NAME_GEN_ALPHA_SIZE * NAME_GEN_ALPHA_SIZE) continue;

                char next = buffer[i + k];
                name_gen_entry *entry = &gen->table[idx];

                // Check if this continuation already exists
                bool found = false;
                for (i32 j = 0; j < entry->num_options; j++) {
                    if (entry->options[j] == next) {
                        if (entry->counts[j] < 255) entry->counts[j]++;
                        found = true;
                        break;
                    }
                }

                // Add new continuation
                if (!found && entry->num_options < 32) {
                    entry->options[entry->num_options] = next;
                    entry->counts[entry->num_options] = 1;
                    entry->num_options++;
                }
            }
        }

        s = e + 1;
        e = strstr(s, ",");
    }

    SDL_free(file_contents);
    printf("[NAME_GEN] Trained on %s\n", file_path);
}

void name_gen_clear(name_gen *gen) {
    if (gen->_mem.base) {
        mem_arena_clear(&gen->_mem);
    }
    gen->table = nullptr;
    gen->num_names = 0;
}

void name_gen_names(name_gen *gen, u32 num) {
    gen->num_names = 0;

    char name[NAME_GEN_BUF_LEN];
    u32 attempts = 0;
    while (gen->num_names < num) {
        attempts++;
        i32 len = name_gen_create(gen, name);
        if (len < 0) continue;

        if (name_gen_validate(name, len)) {
            gen->names[gen->num_names++] = alloc_name(&gen->_mem, name, len);
        }
    }
}

void name_gen_district(name_gen *gen, u32 num) {
    gen->num_names = 0;

    static const char *prefixes[] = {
        "Little", "Grand", "Silver", "High", "Low", "Old", "New", "Upper", "Lower", "Greater"
    };
    static const u32 num_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);

    static const char *suffixes[] = {
        "Heights", "Park", "Hills", "Grove", "Valley", "District", "Quarter",
        "Gardens", "Square", "Town", "Village", "Estates", "Side", "End"
    };
    static const u32 num_suffixes = sizeof(suffixes) / sizeof(suffixes[0]);

    char name[NAME_GEN_BUF_LEN];
    u32 attempts = 0;
    while (gen->num_names < num) {
        attempts++;
        i32 len = name_gen_create(gen, name);
        if (len < 0 || !name_gen_validate(name, len)) continue;

        // 33% chance suffix, 25% chance prefix, rest just base name
        i32 roll = rand_int(12);
        if (roll < 4) {
            // Add suffix
            const char *suffix = suffixes[rand_int(num_suffixes)];
            i32 suf_len = (i32)strlen(suffix);
            if (len + 1 + suf_len < NAME_GEN_BUF_LEN) {
                strcat(name, " ");
                strcat(name, suffix);
                len += 1 + suf_len;
            }
        } else if (roll < 7) {
            // Add prefix
            const char *prefix = prefixes[rand_int(num_prefixes)];
            i32 pre_len = (i32)strlen(prefix);
            if (len + 1 + pre_len < NAME_GEN_BUF_LEN) {
                memmove(name + pre_len + 1, name, len + 1);
                memcpy(name, prefix, pre_len);
                name[pre_len] = ' ';
                len += 1 + pre_len;
            }
        }
        // else: just use base name as-is

        gen->names[gen->num_names++] = alloc_name(&gen->_mem, name, len);
    }
}

void name_gen_street(name_gen *gen, u32 num) {
    gen->num_names = 0;

    static const char *suffixes[] = {
        "Street", "Avenue", "Boulevard", "Road", "Drive", "Lane", "Way",
        "Place", "Court", "Circle", "Terrace", "Trail", "Parkway"
    };
    static const u32 num_suffixes = sizeof(suffixes) / sizeof(suffixes[0]);

    // Weights favor common suffixes (Street, Avenue, Road, Drive)
    static const i32 suffix_weights[] = {
        10, 8, 4, 8, 6, 5, 4, 3, 3, 2, 2, 2, 2
    };

    char name[NAME_GEN_BUF_LEN];
    u32 attempts = 0;
    while (gen->num_names < num) {
        attempts++;
        i32 len = name_gen_create(gen, name);
        if (len < 0 || !name_gen_validate(name, len)) continue;

        // Always add a street suffix
        i32 suffix_idx = rand_weighted_index(rand_float01(), suffix_weights, num_suffixes);
        const char *suffix = suffixes[suffix_idx];
        i32 suf_len = (i32)strlen(suffix);

        if (len + 1 + suf_len < NAME_GEN_BUF_LEN) {
            strcat(name, " ");
            strcat(name, suffix);
            len += 1 + suf_len;
        }

        gen->names[gen->num_names++] = alloc_name(&gen->_mem, name, len);
    }
}
