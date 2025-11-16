#include "qg_random.hpp"
#include <random>

thread_local std::mt19937_64 g_rand_eng;
thread_local std::uniform_real_distribution<float> g_rand_dist;

void rand_seed(int64_t seed) {
    g_rand_eng.seed(seed);
}

float rand_float01() {
    return g_rand_dist(g_rand_eng);
}

int rand_weighted_index(int8_t *weights, int num_items) {
    int sum = 0;
    for (int i = 0; i < num_items; i++) {
        sum += weights[i];
    }

    int target = (int)(rand_float01() * sum) + 1;

    int current = 0;
    while (target > weights[current]) {
        target -= weights[current];
        current++;
    }
    return current;
}
