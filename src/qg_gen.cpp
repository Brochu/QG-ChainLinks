#include "qg_generator.hpp"
#include "qg_random.hpp"
#include "sqlite3.h"

#include <assert.h>
#include <SDL3/SDL.h>

#include "qg_gen_sql.inc" // SQL static command strings

int debug_callback(void *data, int size, char **var0, char **var1) {
    return 0;
}

i8 size_to_districts[city_size::SIZE_COUNT] = { 1, 3, 5, 9 };
i32 district_weights[district_type::DISTRICT_COUNT] = { 2, 3, 4, 1, 1 };

i8 size_to_landmarks[city_size::SIZE_COUNT] = { 5, 15, 40, 80 };
i32 landmark_size_weights[landmark_size::LANDMARK_SIZE_COUNT] = { 1, 3, 4, 2 };

void case_gen_init(case_gen *ctx) {
    //TODO: Memory arena init

    int res = sqlite3_open(":memory:", &ctx->db);
    assert(res == 0 && "Could not open in-memory DB");
    printf("[PROC-GEN] Opening in-memory DB; res = %i\n", res);

    sqlite3_exec(ctx->db, sql_init, debug_callback, NULL, NULL);
}

void case_gen_fondation(case_gen *ctx, city_size s, i32 seed) {
    rand_seed(seed);

    ctx->size = s;
    ctx->num_landmarks = size_to_landmarks[s];
    ctx->num_districts = size_to_districts[s];

    sqlite3_stmt *prep;
    int res = sqlite3_prepare_v3(ctx->db, sql_district, strlen(sql_district), 0, &prep, nullptr);
    if (res != 0) {
        printf("[PROC-GEN] Could not compile statement. ERR: %s\n", sqlite3_errmsg(ctx->db));
        assert(0 && "Could not create new prepared statement");
    }
    for (int i = 0; i < ctx->num_districts; i++) {
        // Districts
        sqlite3_reset(prep);

        district d;
        d.name = "TOWNVILLE";
        d.type = district_type::RESIDENTIAL;
        d.wealth = rand_int(5) + 1;
        d.roughness = rand_int(5) + 1;
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
        sqlite3_reset(prep);

        //TODO: Lot of these will depend on district type and landmark type
        landmark l;
        l.district_id = rand_int(ctx->num_districts)+1;
        l.name = "TOWN SQUARE";
        l.type = landmark_type::LANDMARK_TYPE_HOME;
        l.size = landmark_size::LANDMARK_SIZE_SMALL;
        l.open_hour = 0;
        l.close_hour = 0;
        l.peak_hour = 0;
        l.num_staff = 0;
        l.is_public = 0;
        l.crime_factor = 0;

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

// ====================


bool name_gen_validate(const std::string &name) {
    size_t f_space = name.find_first_of(' ');
    size_t l_space = name.find_last_of(' ');
    if (f_space != l_space) return false; // Make sure we accept only MAX one space in the name

    if (f_space == std::string::npos && (name.size() > 15 || name.size() < 5)) return false; // Prevent single word name over 15 chars or under 5
    if (f_space != std::string::npos && (f_space > 10 || name.size() - f_space > 10)) return false; // Prevent each word in composed name to be over 10 chars

    return true;
}

void name_gen_train(name_gen *gen, const char *file_path) {
    u64 len = 0;
    const char *file_contents = (char*)SDL_LoadFile(file_path, &len);
    std::string data(file_contents);

    u64 s = 0;
    u64 e = data.find(',', s);
    static u64 k = 3;

    while (e != std::string::npos) {
        std::string name = "^^^" + data.substr(s, e-s) + "$$$";

        s = e + 1;
        e = data.find(',', s);

        for (int i = 0; i < name.size() - k; i++) {
            std::string prefix = name.substr(i, k);
            char next = name[i + k];
            gen->counts[prefix][next]++;
        }
    }

    SDL_free((void*)file_contents);
}

void name_gen_next(name_gen *gen, u64 num, std::vector<std::string> *out) {
    static u64 k = 3;
    std::vector<char> options;
    std::vector<int32_t> weights;

    for (u64 i = 0; i < num; i++) {
        std::string name = "^^^";

        char next = '\0';
        while (next != '$') {
            std::string prefix = name.substr(name.size() - k, k);

            options.clear();
            weights.clear();
            for (auto [k, v] : gen->counts[prefix]) {
                options.emplace_back(k);
                weights.emplace_back(v);
            }

            int next_idx = rand_weighted_index(weights.data(), weights.size());
            next = options[next_idx];
            name += next;
        }

        name.erase(0, k);
        name.resize(name.size() - 1);
        if (name_gen_validate(name)) {
            out->push_back(name);
        } else {
            i--;
        }
    }
}

void name_gen_district(name_gen *gen, u64 num, std::vector<std::string> *out) {
    static const char *district_prefix[] { "Little", "Grand", "Silver", "High", "Low", "Old", "New", "Upper", "Lower", "Greater" };
    static u64 num_district_prefix = sizeof(district_prefix) / sizeof(district_prefix[0]);
    static const char *district_suffix[] { "Heights", "Park", "Hills", "Grove", "Valley", "District", "Quarter", "Gardens", "Square", "Town", "Village", "Estates", "Side", "End" };
    static u64 num_district_suffix = sizeof(district_suffix) / sizeof(district_suffix[0]);

    //TODO: Is there a better way
    static i32 s_weights[15] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 42 };
    static i32 p_weights[11] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 30 };

    name_gen_next(gen, num, out);
    for (std::string &name : *out) {
        int suffix_idx = rand_weighted_index(s_weights, 15);
        if (suffix_idx < num_district_suffix) {
            name.append(" ");
            name.append(district_suffix[suffix_idx]);
            continue;
        }

        int prefix_idx = rand_weighted_index(p_weights, 11);
        if (prefix_idx < num_district_prefix) {
            name.insert(0, " ");
            name.insert(0, district_prefix[prefix_idx]);
        }
    }
}

void name_cycle_init(name_cycle *ctx, const char *file_path) {
    static u64 NAME_ARENA_SIZE = 850 * 20;
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
        ctx->list.emplace_back((const char *)pname.p);

        s = e + 1;
        e = data.find(',', s);
    }

    SDL_free((void*)file_contents);

    static u32 prime_steps[] { 2, 29, 73, 113, 179, 229, 283, 349, 419, 463, 547, 601 };
    static u64 num_prime_steps = sizeof(prime_steps) / sizeof(prime_steps[0]);
    static i32 primes_weights[12] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

    ctx->step = prime_steps[rand_weighted_index(primes_weights, num_prime_steps)];
    while (ctx->step % ctx->list.size() == 0) {
        ctx->step++;
    }
    ctx->next = rand_int(ctx->list.size());
}

void name_cycle_next(name_cycle *ctx, u64 num, std::vector<std::string> *out) {
    if (!ctx || !out || ctx->list.size() <= 0) return;

    for (u64 i = 0; i < num; i++) {
        out->emplace_back(ctx->list[ctx->next]);
        ctx->next = (ctx->next + ctx->step) % ctx->list.size();
    }
}

void name_cycle_clear(name_cycle *ctx) {
    ctx->list.clear();
    mem_arena_clear(&ctx->mem);

    ctx->step = 0;
    ctx->next = 0;
}
