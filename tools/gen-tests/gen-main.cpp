#include <cstdio>
#include <cstdint>

struct gen_loc_t {
    int64_t id;
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

enum class gen_type_t : int32_t {
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

    gen_type_t type;
    union {
        struct {
        } crime;
        struct {
        } sighting;
    };
};
static gen_event_t s_events[1024];

//TODO: Define the steps to generate cases
int main(int argc, char **argv) {
    printf("[PROC-GEN] Testing history/events generation & simulation\n");
    return 0;
}
