#pragma once
#include "shared_types.hpp"

void rand_seed(i64 seed);

f32 rand_float01();

i32 rand_int(i32 max_val);
i32 rand_int_min(i32 min_val, i32 max_val);

i8 rand_actor_age();

template<class T>
i32 rand_weighted_index(T *weights, i32 num_items) {
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

template<class T>
i32 rand_weighted_index(f32 roll, T *weights, i32 num_items) {
    i64 sum = 0;
    for (int i = 0; i < num_items; i++) {
        sum += weights[i];
    }

    i64 target = (int)(roll * sum) + 1;

    i32 current = 0;
    while (target > weights[current]) {
        target -= weights[current];
        current++;
    }
    return current;
}

#define RANDOM_MODULE_DEF \
    X(void, rand_seed, (i64)) \
    X(f32, rand_float01, (void)) \
    X(i32, rand_int, (i32)) \
    X(i32, rand_int_min, (i32, i32))  \
    X(i8, rand_actor_age, (void))

struct qg_random_api {
    #define X(ret, name, params) ret (*name) params;
    RANDOM_MODULE_DEF
    #undef X
};
