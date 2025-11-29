#include "qg_random.hpp"
#include <random>

thread_local std::mt19937_64 g_rand_eng;
thread_local std::uniform_real_distribution<f32> g_rand_dist;

void rand_seed(i64 seed) {
    g_rand_eng.seed(seed);
}

f32 rand_float01() {
    return g_rand_dist(g_rand_eng);
}

i32 rand_weighted_index(i32 *weights, i32 num_items) {
    i64 sum = 0;
    for (int i = 0; i < num_items; i++) {
        sum += weights[i];
    }

    i64 target = (int)(rand_float01() * sum) + 1;

    i32 current = 0;
    while (target > weights[current]) {
        target -= weights[current];
        current++;
    }
    return current;
}
