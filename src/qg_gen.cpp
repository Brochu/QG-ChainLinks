#include "qg_generator.hpp"
#include "qg_random.hpp"

#include <SDL3/SDL.h>
#include <sqlite3.h>

int debug_callback(void *data, int size, char **var0, char **var1) {
    return 0;
}

int8_t size_to_districts[city_size::SIZE_COUNT] = { 1, 3, 5, 9 };
int32_t district_weights[district_type::DISTRICT_COUNT] = { 2, 3, 4, 1, 1 };

int8_t size_to_landmarks[city_size::SIZE_COUNT] = { 5, 15, 40, 80 };

void case_gen_fondation(case_gen *ctx, city_size s, int32_t seed) {
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

const char *district_prefix[] { "Little", "Grand", "Silver", "High", "Low", "Old", "New", "Upper", "Lower", "Greater" };
const size_t num_district_prefix = sizeof(district_prefix) / sizeof(district_prefix[0]);
const char *district_suffix[] { "Heights", "Park", "Hills", "Grove", "Valley", "District", "Quarter", "Gardens", "Square", "Town", "Village", "Estates", "Side", "End" };
const size_t num_district_suffix = sizeof(district_suffix) / sizeof(district_suffix[0]);

bool name_gen_validate(const std::string &name) {
    size_t f_space = name.find_first_of(' ');
    size_t l_space = name.find_last_of(' ');
    if (f_space != l_space) return false; // Make sure we accept only MAX one space in the name

    if (f_space == std::string::npos && (name.size() > 15 || name.size() < 5)) return false; // Prevent single word name over 15 chars or under 5
    if (f_space != std::string::npos && (f_space > 10 || name.size() - f_space > 10)) return false; // Prevent each word in composed name to be over 10 chars

    return true;
}

void name_gen_train(name_gen *gen, const char *file_path) {
    size_t len = 0;
    std::string data = (char *)SDL_LoadFile(file_path, &len);

    size_t s = 0;
    size_t e = data.find(',', s);
    static size_t k = 3;

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
}

void name_gen_next(name_gen *gen, size_t num, std::vector<std::string> *out) {
    static size_t k = 3;
    std::vector<char> options;
    std::vector<int32_t> weights;

    for (int i = 0; i < num; i++) {
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

void name_gen_district(name_gen *gen, size_t num, std::vector<std::string> *out) {
    name_gen_next(gen, num, out);

    //TODO: Is there a better way
    int s_weights[15] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 42 };
    int p_weights[11] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 30 };

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
