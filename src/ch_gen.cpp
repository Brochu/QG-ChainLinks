#include "ch_generator.hpp"
#include "qg_random.hpp"
#include "shared.hpp"

#include <sqlite3.h>
#include <SDL3/SDL.h>

#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define GEN_ALPHA_SIZE 58
#define GEN_MAP_SIZE 256
#define NAME_BUF_LEN 64

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
        g_eng.mem_arena_init(&gen->_names_mem, (GEN_ALPHA_SIZE*GEN_ALPHA_SIZE*GEN_ALPHA_SIZE*sizeof(name_gen_entry)) + (MAX_NUM_NAMES * 15 * sizeof(char)));
    } else {
        g_eng.mem_arena_reset(&gen->_names_mem);
    }

    assert(gen->table == nullptr && "Trying to re-train a name_gen instance");
    gen->table = (name_gen_entry *)g_eng.mem_arena_alloc(&gen->_names_mem, GEN_ALPHA_SIZE*GEN_ALPHA_SIZE*GEN_ALPHA_SIZE*sizeof(name_gen_entry), sizeof(void*)).p;

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
    assert(gen->_names_mem.base != nullptr && "Cannot clear an empty name_gen instance");

    g_eng.mem_arena_clear(&gen->_names_mem);
    gen->table = nullptr;
    gen->num_names = 0;
}

bool _name_gen_validate(const char *name, i32 len) {
    i32 f_space = INT32_MAX;
    i32 l_space = INT32_MIN;
    for (i32 i = 0; i < len; i++) {
        if (name[i] == ' ') {
            f_space = std::min(f_space, i);
            l_space = std::max(l_space, i);
        }
    }

    if ((f_space != INT32_MAX || l_space != INT32_MIN) && f_space != l_space) return false; // Make sure we accept only MAX one space in the name

    if (f_space == INT32_MAX && (len > 15 || len < 5)) return false; // Prevent single word name over 15 chars or under 5
    if (f_space != INT32_MAX && (f_space > 10 || len - f_space - 1 > 10)) return false; // Prevent each word in composed name to be over 10 chars

    return true;
}

i32 _name_gen_create(name_gen *gen, char *name) {
    const static u64 k = 3;
    name[0] = name[1] = name[2] = '^';

    char next = '\0';
    i32 i;
    for (i = 0; next != '$'; i++) {
        i32 hi = ascii_to_dense[name[i+0]] * GEN_ALPHA_SIZE * GEN_ALPHA_SIZE;
        i32 mid = ascii_to_dense[name[i+1]] * GEN_ALPHA_SIZE;
        i32 lo = ascii_to_dense[name[i+2]];
        i32 idx = hi + mid + lo;
        if (idx >= GEN_ALPHA_SIZE * GEN_ALPHA_SIZE * GEN_ALPHA_SIZE) {
            printf("euuu nop '%c%c%c'\n", name[i+0], name[i+1], name[i+2]);
        }

        name_gen_entry *e = &gen->table[idx];
        int next_idx = rand_weighted_index(g_eng.rand_float01(), e->counts, e->num_options);
        next = e->options[next_idx];
        name[i+3] = next;
    }

    memmove_s(name, NAME_BUF_LEN, name+3, i);
    name[i-1] = '\0';

    return i-1;
}

void name_gen_next(name_gen *gen, u64 num) {
    gen->num_names = 0;

    char name[NAME_BUF_LEN];
    for (u64 i = 0; i < num; i++) {
        i32 len = _name_gen_create(gen, name);

        if (_name_gen_validate(name, len)) {
            arena_ptr ptr = g_eng.mem_arena_alloc(&gen->_names_mem, len+1, sizeof(void*));
            strcpy_s((char *)ptr.p, len+1, name);
            gen->names[gen->num_names++] = (const char *)ptr.p;
        } else {
            i--;
        }
    }
}

void name_gen_district(name_gen *gen, u64 num) {
    gen->num_names = 0;

    static const char *district_prefix[] { "Little", "Grand", "Silver", "High", "Low", "Old", "New", "Upper", "Lower", "Greater" };
    static u64 num_district_prefix = sizeof(district_prefix) / sizeof(district_prefix[0]);
    static const char *district_suffix[] { "Heights", "Park", "Hills", "Grove", "Valley", "District", "Quarter", "Gardens", "Square", "Town", "Village", "Estates", "Side", "End" };
    static u64 num_district_suffix = sizeof(district_suffix) / sizeof(district_suffix[0]);

    static const i32 prefix_odds_denum = 2;
    static const i32 suffix_odds_denum = 3;

    char name[NAME_BUF_LEN];
    for (u64 i = 0; i < num; i++) {
        i32 len = _name_gen_create(gen, name);

        if (!_name_gen_validate(name, len)) {
            i--;
            continue;
        }

        if (g_eng.rand_int(suffix_odds_denum) == 0) {
            const char *suffix = district_suffix[g_eng.rand_int(num_district_suffix)];
            u64 suf_len = strlen(suffix);
            assert(suf_len + 1 + len < 64 && "Not enough space to append suffix");

            strncat_s(name, NAME_BUF_LEN, " ", 1);
            strncat_s(name, NAME_BUF_LEN, suffix, suf_len);
            len += suf_len+1;
        }
        else if (g_eng.rand_int(prefix_odds_denum) == 0) {
            const char *prefix = district_prefix[g_eng.rand_int(num_district_prefix)];
            u64 pre_len = strlen(prefix);
            assert(pre_len + 1 + len < 64 && "Not enough space to prepend prefix");

            memmove_s(name+pre_len+1, NAME_BUF_LEN-(pre_len+1), name, len);
            name[pre_len] = ' ';
            memcpy_s(name, pre_len, prefix, pre_len);
            len += pre_len+1;

            name[len] = '\0';
        }

        arena_ptr ptr = g_eng.mem_arena_alloc(&gen->_names_mem, len+1, sizeof(void*));
        strcpy_s((char *)ptr.p, len+1, name);
        gen->names[gen->num_names++] = (const char *)ptr.p;
    }
}

void name_cycle_init(name_cycle *ctx, const char *file_path) {
    static u64 NAME_ARENA_SIZE = MAX_NUM_NAMES * 2 * 2 * 20;
    if (ctx->_mem.base == nullptr) {
        g_eng.mem_arena_init(&ctx->_mem, NAME_ARENA_SIZE);
    } else {
        g_eng.mem_arena_reset(&ctx->_mem);
    }

    u64 len = 0;
    char *file_contents = (char*)SDL_LoadFile(file_path, &len);

    char *s = file_contents;
    char *e = strstr(file_contents, ",");

    while (e != NULL) {
        const u64 len = (e-s)+1;
        arena_ptr pname = g_eng.mem_arena_alloc(&ctx->_mem, len, 1);

        memcpy_s(pname.p, len, s, len-1);
        pname.p[len-1] = '\0';
        ctx->names[ctx->num_names++] = (const char *)pname.p;

        s = e + 1;
        e = strstr(s, ",");
    }

    SDL_free((void*)file_contents);

    static u32 prime_steps[] { 2, 29, 73, 113, 179, 229, 283, 349, 419, 463, 547, 601 };
    static u64 num_prime_steps = sizeof(prime_steps) / sizeof(prime_steps[0]);

    ctx->step = prime_steps[g_eng.rand_int(num_prime_steps)];
    while (ctx->step % ctx->num_names == 0) {
        ctx->step++;
    }
    ctx->next = g_eng.rand_int(ctx->num_names);
}

void name_cycle_clear(name_cycle *ctx) {
    g_eng.mem_arena_clear(&ctx->_mem);
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

#include "ch_gen_sql.inc" // SQL static command strings

#define CASEGEN_SCRATCH_ALLOC 2048*2048
#define LANDMARK_POS_MAX 45
#define ASSETS_HOME_DIR "../assets/"

int debug_callback(void *data, int size, char **var0, char **var1) {
    return 0;
}

i32 _case_gen_transit_time(i32 dist, travel_mode mode, travel_phase time) {
    i32 mins_mult = 0;
    i32 mins_add = 0;

    if (mode == travel_mode::TRAVEL_WALK) {
        mins_mult = 4;
        mins_add = (time == travel_phase::TIME_RUSH) ? 2 : 1;
    }
    else if (mode == travel_mode::TRAVEL_DRIVE) {
        mins_mult = (time == travel_phase::TIME_RUSH) ? 2 : 1;
        mins_add = (time == travel_phase::TIME_NIGHT) ? 1 : 2;
    }

    return (dist * mins_mult) + mins_add;
}

void case_gen_init(case_gen *ctx) {
    g_eng.mem_arena_init(&ctx->_scratch, CASEGEN_SCRATCH_ALLOC);

    name_gen_train(&ctx->dist_names, ASSETS_HOME_DIR "city_names.csv");
    name_cycle_init(&ctx->female_names, ASSETS_HOME_DIR "f_names.csv");
    name_cycle_init(&ctx->male_names, ASSETS_HOME_DIR "m_names.csv");

    int res = sqlite3_open(":memory:", &ctx->db);
    assert(res == 0 && "Could not open in-memory DB");
    printf("[PROC-GEN] Opening in-memory DB; res = %i\n", res);

    sqlite3_exec(ctx->db, sql_init, debug_callback, NULL, NULL);
}

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

    name_cycle_clear(&ctx->male_names);
    name_cycle_clear(&ctx->female_names);
    name_gen_clear(&ctx->dist_names);

    g_eng.mem_arena_clear(&ctx->_scratch);
}

static i8 size_to_districts[city_size::SIZE_COUNT] = { 1, 3, 5, 9 };
static i8 size_to_landmarks[city_size::SIZE_COUNT] = { 5, 15, 40, 80 };

static i32 district_weights[district_type::DISTRICT_COUNT] = { 4, 3, 2, 1, 1, 1 };
static i32 landmark_size_weights[landmark_size::LANDMARK_SIZE_COUNT] = { 1, 3, 4, 2 };
static i32 landmark_size_staff[landmark_size::LANDMARK_SIZE_COUNT] = { 2, 6, 20, 60 };

static const char *landmark_type_names[landmark_type::LANDMARK_TYPE_COUNT] {
    "RES_HOUSE", "RES_APARTMENT", "RES_CLINIC", "RES_CORNERSTORE", "RES_PARK", "RES_CHURCH",
    "COMM_RESTAURANT", "COMM_BAR", "COMM_HOTEL", "COMM_STORE", "COMM_BANK", "COMM_APARTMENT",
    "INDU_WAREHOUSE", "INDU_FACTORY", "INDU_ABANDONED",
    "NIGHT_BAR", "NIGHT_NIGHTCLUB", "NIGHT_CASINO", "NIGHT_STRIPCLUB", "NIGHT_FASTFOOD",
    "DOCKS_WAREHOUSE", "DOCKS_SHIPYARD", "DOCKS_BAR", "DOCKS_HOTEL",
    "FIN_BANK", "FIN_OFFICE", "FIN_HOTEL", "FIN_COURTHOUSE",
};

void case_gen_fondation(case_gen *ctx, city_size s, i32 seed) {
    g_eng.rand_seed(seed);

    ctx->size = s;
    ctx->num_districts = size_to_districts[s];
    ctx->districs = (district*)g_eng.mem_arena_alloc(&ctx->_scratch, sizeof(district) * ctx->num_districts, sizeof(u64)).p;

    name_gen_district(&ctx->dist_names, ctx->num_districts);

    sqlite3_stmt *prep;
    int res = sqlite3_prepare_v3(ctx->db, sql_district, strlen(sql_district), 0, &prep, nullptr);
    if (res != 0) {
        printf("[PROC-GEN] Could not compile statement. ERR: %s\n", sqlite3_errmsg(ctx->db));
        assert(0 && "Could not create new prepared statement");
    }
    for (int i = 0; i < ctx->num_districts; i++) {
        sqlite3_reset(prep);

        district &d = ctx->districs[i];
        d.name = ctx->dist_names.names[i];
        d.type = (district_type)rand_weighted_index(g_eng.rand_float01(), district_weights, (i == 0) ? 1 : district_type::DISTRICT_COUNT); // Force RESIDENTIAL for first District
        d.wealth = g_eng.rand_int_min(1, 6);
        d.roughness = g_eng.rand_int_min(1, 6);
        d.response_time = 5 + (10 - d.wealth) + g_eng.rand_int(6);

        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":id"), i);
        sqlite3_bind_text(prep, sqlite3_bind_parameter_index(prep, ":name"), d.name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":type"), d.type);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":wealth"), d.wealth);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":rough"), d.roughness);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":response_time"), d.response_time);
        sqlite3_step(prep);
    }
    res = sqlite3_finalize(prep);
    assert(res == 0 && "Could not finalize prepared statement");

    ctx->num_landmarks = size_to_landmarks[s];
    ctx->landmarks = (landmark*)g_eng.mem_arena_alloc(&ctx->_scratch, sizeof(landmark) * ctx->num_landmarks, sizeof(u64)).p;
    i8 temp_counts[landmark_type::LANDMARK_TYPE_COUNT];
    memset(temp_counts, 0, sizeof(i8) * landmark_type::LANDMARK_TYPE_COUNT);
    char name[64];

    res = sqlite3_prepare_v3(ctx->db, sql_landmark, strlen(sql_landmark), 0, &prep, nullptr);
    if (res != 0) {
        printf("[PROC-GEN] Could not compile statement. ERR: %s\n", sqlite3_errmsg(ctx->db));
        assert(0 && "Could not create new prepared statement");
    }
    for (int i = 0; i < ctx->num_landmarks; i++) {
        sqlite3_reset(prep);

        landmark &l = ctx->landmarks[i];
        l.district_id = g_eng.rand_int(ctx->num_districts);

        district &dist = ctx->districs[l.district_id];
        switch (dist.type) {
        case district_type::RESIDENTIAL:
            l.type = (landmark_type)g_eng.rand_int_min(landmark_type::LANDMARK_TYPE_RES_START, LANDMARK_TYPE_RES_END);
            break;

        case district_type::COMMERCIAL:
            l.type = (landmark_type)g_eng.rand_int_min(landmark_type::LANDMARK_TYPE_COMM_START, LANDMARK_TYPE_COMM_END);
            break;

        case district_type::INDUSTRIAL:
            l.type = (landmark_type)g_eng.rand_int_min(landmark_type::LANDMARK_TYPE_INDU_START, LANDMARK_TYPE_INDU_END);
            break;

        case district_type::NIGHTLIFE:
            l.type = (landmark_type)g_eng.rand_int_min(landmark_type::LANDMARK_TYPE_NIGHT_START, LANDMARK_TYPE_NIGHT_END);
            break;

        case district_type::DOCKS:
            l.type = (landmark_type)g_eng.rand_int_min(landmark_type::LANDMARK_TYPE_DOCKS_START, LANDMARK_TYPE_DOCKS_END);
            break;

        case district_type::FINANCIAL:
            l.type = (landmark_type)g_eng.rand_int_min(landmark_type::LANDMARK_TYPE_FIN_START, LANDMARK_TYPE_FIN_END);
            break;

        case district_type::DISTRICT_COUNT:
            assert(false && "[PROC-GEN] Invalid district type provided");
            break;;
        }
        sprintf_s(name, "%s - %i", landmark_type_names[l.type], temp_counts[l.type]++);
        l.name = name;

        l.size = (landmark_size)g_eng.rand_int(landmark_size::LANDMARK_SIZE_COUNT); // Maybe we want to limit max size based on district's wealth
        l.open_hour = 0;
        l.close_hour = 0;
        l.peak_hour = 0;
        l.num_staff = landmark_size_staff[l.size] + g_eng.rand_int_min(-2, 2);

        bool is_public =
            l.type != LANDMARK_TYPE_DOCKS_WAREHOUSE &&
            l.type != LANDMARK_TYPE_INDU_WAREHOUSE &&
            l.type != LANDMARK_TYPE_INDU_ABANDONED &&
            l.type != LANDMARK_TYPE_FIN_OFFICE;
        l.is_public = is_public;

        i32 crime_f = std::max(0, (dist.roughness * 5) + g_eng.rand_int_min(-10, 10));
        if (dist.type == district_type::NIGHTLIFE || dist.type == district_type::FINANCIAL) {
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

    i8 xs[128];
    i8 ys[128];
    for (int i = 0; i < ctx->num_landmarks; i++) {
        xs[i] = g_eng.rand_int(LANDMARK_POS_MAX);
        ys[i] = g_eng.rand_int(LANDMARK_POS_MAX);
    }

    res = sqlite3_prepare_v3(ctx->db, sql_transit, strlen(sql_transit), 0, &prep, nullptr);
    if (res != 0) {
        printf("[PROC-GEN] Could not compile statement. ERR: %s\n", sqlite3_errmsg(ctx->db));
        assert(0 && "Could not create new prepared statement");
    }
    for (int i = 0; i < ctx->num_landmarks; i++) {
        for (int j = 0; j < ctx->num_landmarks; j++) {
            if (i == j) {
                continue;
            }
            const i32 dx = xs[j] - xs[i];
            const i32 dy = ys[j] - ys[i];
            const i32 dist = sqrt(dx*dx + dy*dy);

            for (int mode = 0; mode < travel_mode::TRAVEL_COUNT; mode++) {
                for (int phase = 0; phase < travel_phase::TIME_COUNT; phase++) {
                    sqlite3_reset(prep);

                    i32 mins = _case_gen_transit_time(dist, (travel_mode)mode, (travel_phase)phase);
                    sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":from_landmark_id"), i+1);
                    sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":to_landmark_id"), j+1);
                    sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":mode"), mode);
                    sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":day_phase"), phase);
                    sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":minutes"), std::max(1, g_eng.rand_int_min(mins-1, mins+1)));
                    sqlite3_step(prep);
                }
            }
        }
    }
    res = sqlite3_finalize(prep);
    assert(res == 0 && "Could not finalize prepared statement");

    res = sqlite3_exec(ctx->db, sql_prune_transit, debug_callback, NULL, NULL);
    assert(res == 0 && "Could not cull 10% of transit entries");
}

void case_gen_population(case_gen *ctx) {
    ctx->num_actors = (ctx->num_landmarks * 3) + (ctx->num_districts * 5);
    ctx->actors = (actor*)g_eng.mem_arena_alloc(&ctx->_scratch, sizeof(actor) * ctx->num_actors, sizeof(u64)).p;

    sqlite3_stmt *prep;
    int res = sqlite3_prepare_v3(ctx->db, sql_actor, strlen(sql_actor), 0, &prep, nullptr);
    if (res != 0) {
        printf("[PROC-GEN] Could not compile statement. ERR: %s\n", sqlite3_errmsg(ctx->db));
        assert(0 && "Could not create new prepared statement");
    }
    for (i32 i = 0; i < ctx->num_actors; i++) {
        sqlite3_reset(prep);

        actor &a = ctx->actors[i];
        a.id = i;
        if (g_eng.rand_int(2) == 0) {
            a.sex = 'M';
            a.name = name_cycle_next(&ctx->male_names);
        } else {
            a.sex = 'F';
            a.name = name_cycle_next(&ctx->female_names);
        }
        a.age = g_eng.rand_actor_age();
        a.job = " -- ";
        a.home_district_id = g_eng.rand_int(ctx->num_districts)+1; //TODO: Weight districts based on type
        a.workplace_landmark_id = g_eng.rand_int(ctx->num_landmarks)+1; //TODO: Weight landmarks based on type
        a.wealth = g_eng.rand_int_min(1, 6);
        a.secrets = 0; // Init empty

        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":actor_id"), i);
        sqlite3_bind_text(prep, sqlite3_bind_parameter_index(prep, ":name"), a.name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":age"), a.age);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":sex"), a.sex);
        sqlite3_bind_text(prep, sqlite3_bind_parameter_index(prep, ":job"), a.job, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":home_district_id"), a.home_district_id);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":workplace_landmark_id"), a.workplace_landmark_id);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":wealth"), a.wealth);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":secrets"), a.secrets);
        sqlite3_step(prep);
    }
    res = sqlite3_finalize(prep);
    assert(res == 0 && "Could not finalize prepared statement");

//static const char *sql_actor_routine = "INSERT INTO actor_routines VALUES (:actor_id, :hour, :landmark_id)";
    res = sqlite3_prepare_v3(ctx->db, sql_actor_routine, strlen(sql_actor_routine), 0, &prep, nullptr);
    if (res != 0) {
        printf("[PROC-GEN] Could not compile statement. ERR: %s\n", sqlite3_errmsg(ctx->db));
        assert(0 && "Could not create new prepared statement");
    }
    for (i32 i = 0; i < ctx->num_actors; i++) {
        sqlite3_reset(prep);

        //TODO: Generate time tables based on home district and work landmark
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":actor_id"), i);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":hour"), 8);
        sqlite3_bind_int(prep, sqlite3_bind_parameter_index(prep, ":landmark_id"), 1);
        sqlite3_step(prep);
    }
    res = sqlite3_finalize(prep);
    assert(res == 0 && "Could not finalize prepared statement");

    res = sqlite3_prepare_v3(ctx->db, sql_actor_routine_variant, strlen(sql_actor_routine_variant), 0, &prep, nullptr);
    if (res != 0) {
        printf("[PROC-GEN] Could not compile statement. ERR: %s\n", sqlite3_errmsg(ctx->db));
        assert(0 && "Could not create new prepared statement");
    }
    for (i32 i = 0; i < ctx->num_actors; i++) {
        //TODO: Generate optional steps for entertainmenet with probabilities
    }
    res = sqlite3_finalize(prep);
    assert(res == 0 && "Could not finalize prepared statement");
}

void case_gen_motive(case_gen *ctx) { }
void case_gen_crime(case_gen *ctx) { }
void case_gen_planning(case_gen *ctx) { }
void case_gen_exec(case_gen *ctx) { }
void case_gen_hook(case_gen *ctx) { }
void case_gen_polish(case_gen *ctx) { }
