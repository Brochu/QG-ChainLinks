#include <cstdio>
#include <cstdint>

#include "sqlite3.h"

enum class loc_type_t : int32_t {
    LOCATION_NONE,
    LOCATION_RESTAURANT,
    LOCATION_BANK,
    LOCATION_COUNT,
};

struct gen_loc_t {
    int64_t id;
    const char *name;
    loc_type_t type;
};

struct gen_traits_t {
    int32_t greed;
    int32_t aggro;
    int32_t loyal;
    int32_t honest;
    int32_t ambition;
    int32_t impluses;
};
//TODO: Define value ranges for traits

struct gen_char_t {
    int64_t id;
    gen_traits_t traits;
};

enum class event_type_t : int32_t {
    EVENT_NONE,
    EVENT_CRIME,
    EVENT_SIGHTING,
    EVENT_COUNT,
};

struct gen_event_t {
    int64_t id;
    int64_t sim_step;
    int64_t next_idx[8] = { 0 };
    size_t num_next = 0;

    event_type_t type;
    union {
        struct {
        } crime;
        struct {
        } sighting;
    };
};
static gen_event_t s_events[1024];
static size_t s_num_events = 0;

static int debug_callback(void *ptr, int n, char **text0, char **text1) {
    return 0;
}

//TODO: Define the steps to generate cases
int main(int argc, char **argv) {
    printf("[PROC-GEN] Testing history/events generation & simulation\n");
    s_events[s_num_events++] = { 0 };

    sqlite3 *db;
    int res = sqlite3_open("./detective.db", &db);
    printf("[PROC-GEN] Opening Case DB; res = %i\n", res);

    char *err = nullptr;
    res = sqlite3_exec(db, "SELECT * FROM users;", debug_callback, NULL, &err);
    printf("[PROC-GEN] EXEC res = %i; err = %s\n", res, err);

    res = sqlite3_close_v2(db);
    printf("[PROC-GEN] Closing Case DB; res = %i\n", res);
    printf("[PROC-GEN] Just testing, SQLITE_OK value = %i\n", SQLITE_OK);

    return 0;
}
