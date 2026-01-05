#pragma once
#include "shared_types.hpp"

struct config {
    i32 value = 69;
};

extern config *root_cfg;

void config_init(config *c, const char *file);
void config_free(config *c);
