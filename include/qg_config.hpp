#pragma once


struct config {
};

extern config root_cfg;

void config_init(config *c);
void config_clear(config *c);
