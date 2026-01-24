# Engine Code Changes

This file tracks modifications made to the local copy of engine code in `cc-casegen/engine/`.
These changes may be candidates for inclusion in the main engine code.

---

## 2026-01-22: Initial Copy

### Files Copied (Unchanged from source)
- `qg_memory.hpp` / `qg_mem.cpp` - Memory management (arena allocator)
- `qg_random.hpp` / `qg_rand.cpp` - Random number generation
- `qg_parse.hpp` / `qg_parse.cpp` - String parsing utilities
- `qg_config.hpp` / `qg_conf.cpp` - Configuration file loading
- `qg_bus.hpp` / `qg_bus.cpp` - Event bus system
- `shared_types.hpp` - Type aliases

### Minor Fix Applied

#### `qg_conf.cpp`
- Changed loop variable from `int i` to `u64 i` in `config_read()` to match `num_entries` type

---

## Engine Functionality Summary

### qg_memory (Memory Management)
- `qg_malloc/calloc/realloc/free` - Standard allocation wrappers
- `mem_arena` - Linear arena allocator with generation tracking
- `arena_ptr` / `arena_off` - Pointer and offset types with generation for stale detection
- `mem_arena_init/reset/clear` - Arena lifecycle
- `mem_arena_alloc/offloc` - Allocation functions

### qg_random (Random Number Generation)
- `rand_seed(i64)` - Seed the RNG
- `rand_float01()` - Random float [0, 1)
- `rand_int(max)` - Random int [0, max)
- `rand_int_min(min, max)` - Random int [min, max)
- `rand_actor_age()` - Normal distribution for actor ages (18-110)
- `rand_weighted_index()` - Weighted random selection (template)

### qg_parse (String Parsing)
- `strview` - Non-owning string view with ptr + len
- `SV_FMT` / `SV_ARG()` - Printf macros for strview
- `sv()` - Create strview from C string
- `sv_find()` - Find substring
- `sv_split()` - Split into array
- `sv_split_once()` - Split at first delimiter

### qg_config (Configuration)
- `config_value` - Union type (SINGLE, RANGE, ARRAY, STRING)
- `config` - Key-value store with arena backing
- `config_init(file)` - Load from file (uses SDL_LoadFile)
- `config_free()` - Cleanup
- `config_read(key)` - Lookup value

### qg_bus (Event Bus)
- `event_type` - Enum for event types (engine 0-499, game 512+)
- `event_bus` - Ring buffer queue with handlers per type
- `bus_init/free` - Lifecycle
- `bus_subscribe/unsubscribe` - Handler registration with generation-based IDs
- `bus_fire` - Queue an event
- `bus_process` - Dispatch all queued events
