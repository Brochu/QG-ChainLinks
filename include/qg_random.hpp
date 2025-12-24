#pragma once

#include "qg_types.hpp"

void rand_seed(i64 seed);
f32 rand_float01();
i32 rand_int(i32 max_val);
i32 rand_int_min(i32 min_val, i32 max_val);

i32 rand_weighted_index(i8 *weights, i32 num_items);
i32 rand_weighted_index(u8 *weights, i32 num_items);
i32 rand_weighted_index(i32 *weights, i32 num_items);
