#pragma once
#include "shared.hpp"

struct config {
    i32 value = 69;
};

extern config *root_cfg;

void config_init(config *c, const char *file);
void config_free(config *c);

#define CONFIG_MODULE_DEF \
    X(void, config_init, (config*, const char*)) \
    X(void, config_free, (config*))

struct qg_config_api {
    #define X(ret, name, params) ret (*name) params;
    CONFIG_MODULE_DEF
    #undef X
};
