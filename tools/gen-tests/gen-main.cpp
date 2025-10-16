#include <cstdio>
#include <cstdint>

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

struct gen_event_t {
    int64_t id;
};
//TODO: Find out to represent a graph of events w/ temporal dependencies

int main(int argc, char **argv) {
    printf("[PROC-GEN] Testing history/events generation & simulation\n");
    return 0;
}
