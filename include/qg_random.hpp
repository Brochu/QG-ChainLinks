#pragma once

#include <cstdint>

void rand_seed(int64_t seed);
float rand_float01();

int rand_weighted_index(int32_t *weights, int num_items);
