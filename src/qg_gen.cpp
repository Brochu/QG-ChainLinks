#include "SDL3/SDL_stdinc.h"
#include "qg_generator.hpp"
#include "qg_random.hpp"

#include <SDL3/SDL.h>
#include <sqlite3.h>

int debug_callback(void *data, int size, char **var0, char **var1) {
    return 0;
}

i8 size_to_districts[city_size::SIZE_COUNT] = { 1, 3, 5, 9 };
i32 district_weights[district_type::DISTRICT_COUNT] = { 2, 3, 4, 1, 1 };

i8 size_to_landmarks[city_size::SIZE_COUNT] = { 5, 15, 40, 80 };

void case_gen_fondation(case_gen *ctx, city_size s, i32 seed) {
    srand(seed);
    ctx->size = s;
    ctx->num_landmarks = 0;

    ctx->num_districts = size_to_districts[s];
    for (int i = 0; i < ctx->num_districts; i++) {
        // Districts
        printf("[PROC-GEN] Generate a new district\n");
        district *d = &ctx->districts[i];
        d->id = i;
        d->type = district_type::RESIDENTIAL;

        for (int j = 0; j < size_to_landmarks[s]; j++) {
            // landmarks
            printf("[PROC-GEN] Generate a new landmark\n");
            landmark *loc = &ctx->landmarks[ctx->num_landmarks++];
            loc->id = ctx->num_landmarks - 1;
            loc->district = i;
        }
    }

    // Transit Links
    for (int i = 0; i < ctx->num_landmarks; i++) {
        for (int j = 0; j < ctx->num_landmarks; j++) {
        }
    }
    // Cut some links

    sqlite3 *db;
    int res = sqlite3_open(":memory:", &db);
    printf("[PROC-GEN] Opening in-memory DB; res = %i\n", res);
    res = sqlite3_close(db);
    printf("[PROC-GEN] Closing in-memory DB; res = %i\n", res);
    /*
    sqlite3 *db;
    int res = sqlite3_open("./detective.db", &db);
    printf("[PROC-GEN] Opening Case DB; res = %i\n", res);

    char *err = nullptr;
    res = sqlite3_exec(db, "SELECT * FROM users;", debug_callback, NULL, &err);
    printf("[PROC-GEN] EXEC res = %i; err = %s\n", res, err);

    res = sqlite3_close_v2(db);
    printf("[PROC-GEN] Closing Case DB; res = %i\n", res);
    printf("[PROC-GEN] Just testing, SQLITE_OK value = %i\n", SQLITE_OK);
    */
}

void case_gen_population(case_gen *ctx) { }
void case_gen_motive(case_gen *ctx) { }
void case_gen_crime(case_gen *ctx) { }
void case_gen_planning(case_gen *ctx) { }
void case_gen_exec(case_gen *ctx) { }
void case_gen_hook(case_gen *ctx) { }
void case_gen_polish(case_gen *ctx) { }

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
    ctx->next = SDL_rand(ctx->list.size());
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
