#pragma once

struct config {
};

extern config *root_cfg;

void config_init(config *c, const char *file);
void config_free(config *c);
