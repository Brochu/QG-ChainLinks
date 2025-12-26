#include "qg_generator.hpp"
#include "qg_memory.hpp"
#include "qg_random.hpp"
#include "sqlite3.h"

#include <array>
#include <assert.h>
#include <cstring>
#include <string>
#include <SDL3/SDL.h>

#define GEN_ALPHA_SIZE 58
#define GEN_MAP_SIZE 256

constexpr std::array<u8, GEN_MAP_SIZE> make_ascii_to_dense() {
    std::array<u8, GEN_MAP_SIZE> arr {};
    for (int i = 0; i < GEN_MAP_SIZE; i++) {
        arr[i] = 0xFF; // Fill with invalid values first
    }

    for (int c = 'A'; c <= 'Z'; c++) {
        arr[c] = c - 'A';
    }
    for (int c = 'a'; c <= 'z'; c++) {
        arr[c] = 26 + (c - 'a');
    }

    arr['^'] = 52;
    arr['$'] = 53;
    arr[' '] = 54;
    arr['.'] = 55;
    arr['-'] = 56;
    arr['\''] = 57;
    return arr;
}
constexpr std::array<u8, GEN_MAP_SIZE> ascii_to_dense = make_ascii_to_dense();

void name_gen_train(name_gen *gen, const char *file_path) {
    if (gen->_names_mem.base == nullptr) {
        mem_arena_init(&gen->_names_mem, MAX_NUM_NAMES * 15 * sizeof(char));
    } else {
        mem_arena_reset(&gen->_names_mem);
    }

    assert(gen->table == nullptr && "Trying to re-train a name_gen instance");
    gen->table = (name_gen_entry*)calloc(GEN_ALPHA_SIZE*GEN_ALPHA_SIZE*GEN_ALPHA_SIZE, sizeof(name_gen_entry));

    u64 len = 0;
    char *file_contents = (char*)SDL_LoadFile(file_path, &len);

    char *s = file_contents;
    char *e = strstr(s, ",");
    const static u64 k = 3;

    while (e != NULL) {
        char buffer[64] = "^^^";
        strncat_s(buffer, s, (e-s));
        strncat_s(buffer, "$$$", 3);
        int buf_len = (e-s) + 6;

        s = e + 1;
        e = strstr(s, ",");

        for (int i = 0; i < buf_len - k; i++) {
            i32 hi = ascii_to_dense[buffer[i+0]] * GEN_ALPHA_SIZE * GEN_ALPHA_SIZE;
            i32 mid = ascii_to_dense[buffer[i+1]] * GEN_ALPHA_SIZE;
            i32 lo = ascii_to_dense[buffer[i+2]];
            i32 idx = hi + mid + lo;
            assert(idx < GEN_ALPHA_SIZE * GEN_ALPHA_SIZE * GEN_ALPHA_SIZE);
            char next = buffer[i+k];

            name_gen_entry *e = &gen->table[idx];
            bool added = false;
            for (int i = 0; i < e->num_options; i++) {
                if (next == e->options[i]) {
                    added = true;
                    if (e->counts[i] < 255) {
                        e->counts[i]++;
                    }
                    break;
                }
            }
            if (!added) {
                assert(e->num_options < 32 && "Ran out of space for possible continuations");
                e->options[e->num_options] = next;
                e->counts[e->num_options++] = 1;
            }
        }
    }

    SDL_free((void*)file_contents);
}

void name_gen_clear(name_gen *gen) {
    assert(gen->table != nullptr && "Cannot clear an empty name_gen instance");
    free(gen->table);
    gen->table = nullptr;

    mem_arena_clear(&gen->_names_mem);
    gen->num_names = 0;
}

bool _name_gen_validate(const std::string &name) {
    size_t f_space = name.find_first_of(' ');
    size_t l_space = name.find_last_of(' ');
    if (f_space != l_space) return false; // Make sure we accept only MAX one space in the name

    if (f_space == std::string::npos && (name.size() > 15 || name.size() < 5)) return false; // Prevent single word name over 15 chars or under 5
    if (f_space != std::string::npos && (f_space > 10 || name.size() - f_space > 10)) return false; // Prevent each word in composed name to be over 10 chars

    return true;
}

void _name_gen_create(name_gen *gen, std::string &name) {
    const static u64 k = 3;
    name.clear();
    name = "^^^";

    char next = '\0';
    while (next != '$') {
        std::string prefix = name.substr(name.size() - k, k);

        i32 hi = ascii_to_dense[prefix[0]] * GEN_ALPHA_SIZE * GEN_ALPHA_SIZE;
        i32 mid = ascii_to_dense[prefix[1]] * GEN_ALPHA_SIZE;
        i32 lo = ascii_to_dense[prefix[2]];
        i32 idx = hi + mid + lo;
        if (idx >= GEN_ALPHA_SIZE * GEN_ALPHA_SIZE * GEN_ALPHA_SIZE) {
            printf("euuu nop '%s'\n", prefix.c_str());
        }

        name_gen_entry *e = &gen->table[idx];
        int next_idx = rand_weighted_index(e->counts, e->num_options);
        next = e->options[next_idx];
        name += next;
    }

    name.erase(0, k);
    name.resize(name.size() - 1);
}

void name_gen_next(name_gen *gen, u64 num) {
    std::string name;
    for (u64 i = 0; i < num; i++) {
        _name_gen_create(gen, name);

        if (_name_gen_validate(name)) {
            arena_ptr ptr = mem_arena_alloc(&gen->_names_mem, name.size()+1);
            strcpy_s((char *)ptr.p, name.size()+1, name.c_str());
            gen->names[gen->num_names++] = (const char *)ptr.p;
        } else {
            i--;
        }
    }
}

void name_gen_district(name_gen *gen, u64 num) {
    static const char *district_prefix[] { "Little", "Grand", "Silver", "High", "Low", "Old", "New", "Upper", "Lower", "Greater" };
    static u64 num_district_prefix = sizeof(district_prefix) / sizeof(district_prefix[0]);
    static const char *district_suffix[] { "Heights", "Park", "Hills", "Grove", "Valley", "District", "Quarter", "Gardens", "Square", "Town", "Village", "Estates", "Side", "End" };
    static u64 num_district_suffix = sizeof(district_suffix) / sizeof(district_suffix[0]);

    static const i32 prefix_odds_denum = 2;
    static const i32 suffix_odds_denum = 2;

    std::string name;
    for (u64 i = 0; i < num; i++) {
        _name_gen_create(gen, name);

        if (!_name_gen_validate(name)) {
            i--;
            continue;
        }

        if (rand_int(suffix_odds_denum) == 0) {
            name.append(" ");
            name.append(district_suffix[rand_int(num_district_suffix)]);
        }
        else if (rand_int(prefix_odds_denum) == 0) {
            name.insert(0, " ");
            name.insert(0, district_prefix[rand_int(num_district_prefix)]);
        }

        arena_ptr ptr = mem_arena_alloc(&gen->_names_mem, name.size()+1);
        strcpy_s((char *)ptr.p, name.size()+1, name.c_str());
        gen->names[gen->num_names++] = (const char *)ptr.p;
    }
}

void name_cycle_init(name_cycle *ctx, const char *file_path) {
    static u64 NAME_ARENA_SIZE = MAX_NUM_NAMES * 2 * 2 * 20;
    mem_arena_init(&ctx->mem, NAME_ARENA_SIZE);

    u64 len = 0;
    const char *file_contents = (char*)SDL_LoadFile(file_path, &len);
    std::string data(file_contents);

    u64 s = 0;
    u64 e = data.find(',', s);

    while (e != std::string::npos) {
        const u64 len = (e-s)+1;
        arena_ptr pname = mem_arena_alloc(&ctx->mem, len, 1);

        memcpy_s(pname.p, len, data.data() + s, len-1);
        pname.p[len-1] = '\0';
        ctx->names[ctx->num_names++] = (const char *)pname.p;

        s = e + 1;
        e = data.find(',', s);
    }

    SDL_free((void*)file_contents);

    static u32 prime_steps[] { 2, 29, 73, 113, 179, 229, 283, 349, 419, 463, 547, 601 };
    static u64 num_prime_steps = sizeof(prime_steps) / sizeof(prime_steps[0]);

    ctx->step = prime_steps[rand_int(num_prime_steps)];
    while (ctx->step % ctx->num_names == 0) {
        ctx->step++;
    }
    ctx->next = rand_int(ctx->num_names);
}

void name_cycle_clear(name_cycle *ctx) {
    mem_arena_clear(&ctx->mem);
    ctx->num_names = 0;

    ctx->step = 0;
    ctx->next = 0;
}

const char *name_cycle_next(name_cycle *ctx) {
    if (!ctx || ctx->num_names <= 0) return nullptr;

    const char *selected = ctx->names[ctx->next];
    ctx->next = (ctx->next + ctx->step) % ctx->num_names;

    return selected;
}

// ====================

#include "qg_gen_sql.inc" // SQL static command strings

int debug_callback(void *data, int size, char **var0, char **var1) {
    return 0;
}

void case_gen_init(case_gen *ctx) {
    //TODO: Memory arena init
    name_gen_train(&ctx->dist_names, "../assets/city_names.csv");

    int res = sqlite3_open(":memory:", &ctx->db);
    assert(res == 0 && "Could not open in-memory DB");
    printf("[PROC-GEN] Opening in-memory DB; res = %i\n", res);

    sqlite3_exec(ctx->db, sql_init, debug_callback, NULL, NULL);
}

static i8 size_to_districts[city_size::SIZE_COUNT] = { 1, 3, 5, 9 };
static i8 size_to_landmarks[city_size::SIZE_COUNT] = { 5, 15, 40, 80 };

static i32 district_weights[district_type::DISTRICT_COUNT] = { 4, 3, 2, 1, 1, 1 };
static i32 landmark_size_weights[landmark_size::LANDMARK_SIZE_COUNT] = { 1, 3, 4, 2 };
static i32 landmark_size_staff[landmark_size::LANDMARK_SIZE_COUNT] = { 2, 6, 20, 60 };

void case_gen_fondation(case_gen *ctx, city_size s, i32 seed) {
    rand_seed(seed);

    ctx->size = s;
    ctx->num_districts = size_to_districts[s];
    ctx->num_landmarks = size_to_landmarks[s];

    name_gen_next(&ctx->dist_names, ctx->num_districts);
    district dist_cache[16];

    sqlite3_stmt *prep;
    int res = sqlite3_prepare_v3(ctx->db, sql_district, strlen(sql_district), 0, &prep, nullptr);
    if (res != 0) {
        printf("[PROC-GEN] Could not compile statement. ERR: %s\n", sqlite3_errmsg(ctx->db));
        assert(0 && "Could not create new prepared statement");
    }
    for (int i = 0; i < ctx->num_districts; i++) {
        // Districts
        sqlite3_reset(prep);

        district &d = dist_cache[i];
        d.name = ctx->dist_names.names[i];
        d.type = (district_type)rand_weighted_index(district_weights, district_type::DISTRICT_COUNT);
        d.wealth = rand_int_min(1, 5);
        d.roughness = rand_int_min(1, 5);
        d.response_time = 5 + (10 - d.wealth) + rand_int(6);

        //sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, "id"), i);
        sqlite3_bind_text(prep, sqlite3_bind_parameter_index(prep, ":name"), d.name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":type"), d.type);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":wealth"), d.wealth);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":rough"), d.roughness);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":response_time"), d.response_time);
        sqlite3_step(prep);
    }
    res = sqlite3_finalize(prep);
    assert(res == 0 && "Could not finalize prepared statement");

    res = sqlite3_prepare_v3(ctx->db, sql_landmark, strlen(sql_landmark), 0, &prep, nullptr);
    if (res != 0) {
        printf("[PROC-GEN] Could not compile statement. ERR: %s\n", sqlite3_errmsg(ctx->db));
        assert(0 && "Could not create new prepared statement");
    }
    for (int i = 0; i < ctx->num_landmarks; i++) {
        // Landmarks
        sqlite3_reset(prep);

        //TODO: Lot of these will depend on district type and landmark type
        landmark l;
        l.district_id = rand_int(ctx->num_districts)+1;
        l.name = "NAME_TBD";

        district &dist = dist_cache[l.district_id-1];
        switch (dist.type) {
        case district_type::RESIDENTIAL:
            l.type = (landmark_type)rand_int_min(landmark_type::LANDMARK_TYPE_RES_START, LANDMARK_TYPE_RES_END);
            break;

        case district_type::COMMERCIAL:
            l.type = (landmark_type)rand_int_min(landmark_type::LANDMARK_TYPE_COMM_START, LANDMARK_TYPE_COMM_END);
            break;

        case district_type::INDUSTRIAL:
            l.type = (landmark_type)rand_int_min(landmark_type::LANDMARK_TYPE_INDU_START, LANDMARK_TYPE_INDU_END);
            break;

        case district_type::NIGHTLIFE:
            l.type = (landmark_type)rand_int_min(landmark_type::LANDMARK_TYPE_NIGHT_START, LANDMARK_TYPE_NIGHT_END);
            break;

        case district_type::DOCKS:
            l.type = (landmark_type)rand_int_min(landmark_type::LANDMARK_TYPE_DOCKS_START, LANDMARK_TYPE_DOCKS_END);
            break;

        case district_type::FINANCIAL:
            l.type = (landmark_type)rand_int_min(landmark_type::LANDMARK_TYPE_FIN_START, LANDMARK_TYPE_FIN_END);
            break;

        case district_type::DISTRICT_COUNT:
            assert(false && "[PROC-GEN] Invalid district type provided");
            break;;
        }

        l.size = (landmark_size)rand_int(landmark_size::LANDMARK_SIZE_COUNT); // Maybe we want to limit max size based on district's wealth
        l.open_hour = 0;
        l.close_hour = 0;
        l.peak_hour = 0;
        l.num_staff = landmark_size_staff[l.size] + rand_int_min(-2, 2);

        bool is_public =
            l.type != LANDMARK_TYPE_DOCKS_WAREHOUSE &&
            l.type != LANDMARK_TYPE_INDU_WAREHOUSE &&
            l.type != LANDMARK_TYPE_INDU_ABANDONED &&
            l.type != LANDMARK_TYPE_FIN_OFFICE;
        l.is_public = is_public;

        i32 crime_f = std::max(0, (dist.roughness * 5) + rand_int_min(-10, 10));
        if (dist.type == district_type::NIGHTLIFE || district_type::FINANCIAL) {
            crime_f *= 2;
        }
        l.crime_factor = crime_f;

        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":district_id"), l.district_id);
        sqlite3_bind_text(prep, sqlite3_bind_parameter_index(prep, ":name"), l.name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":type"), l.type);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":size"), l.size);
        //sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":open_hour"), l.open_hour);
        //sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":close_hour"), l.close_hour);
        //sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":peak_hour"), l.peak_hour);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":num_staff"), l.num_staff);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":is_public"), l.is_public ? 1 : 0);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":crime_factor"), l.crime_factor);
        sqlite3_step(prep);
    }
    res = sqlite3_finalize(prep);
    assert(res == 0 && "Could not finalize prepared statement");

    // Transit Links
    for (int i = 0; i < ctx->num_landmarks; i++) {
        for (int j = 0; j < ctx->num_landmarks; j++) {
        }
    }
    // Cut some links
}

void case_gen_population(case_gen *ctx) { }
void case_gen_motive(case_gen *ctx) { }
void case_gen_crime(case_gen *ctx) { }
void case_gen_planning(case_gen *ctx) { }
void case_gen_exec(case_gen *ctx) { }
void case_gen_hook(case_gen *ctx) { }
void case_gen_polish(case_gen *ctx) { }

void case_gen_clear(case_gen *ctx) {
    {
        //TODO: Make this backup process optional? only happen on errors later in the process?
        sqlite3 *file;
        int res = sqlite3_open("detective.db", &file);
        assert(res == 0 && "Could not open file to save in-memory db");

        sqlite3_backup *back = sqlite3_backup_init(file, "main", ctx->db, "main");
        assert(back != NULL && "Could not init backup process");
        sqlite3_backup_step(back, -1);
        sqlite3_backup_finish(back);

        res = sqlite3_close(file);
        assert(res == 0 && "Could not close in-memory db backup file");
    }

    int res = sqlite3_close(ctx->db);
    assert(res == 0 && "Could not open in-memory DB");
    printf("[PROC-GEN] Closing in-memory DB; res = %i\n", res);

    //TODO: Memory arena cleanup
}
