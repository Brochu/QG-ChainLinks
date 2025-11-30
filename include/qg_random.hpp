#pragma once

#include "qg_types.hpp"

void rand_seed(i64 seed);
f32 rand_float01();
i32 rand_int(i32 max_val);

i32 rand_weighted_index(i32 *weights, i32 num_items);
