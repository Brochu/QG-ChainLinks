#pragma once

#include "qg_types.hpp"

#define CHAIN_API __declspec(dllexport)

extern "C" void CHAIN_API chain_init();
extern "C" void CHAIN_API chain_tick(f32 dt);
extern "C" void CHAIN_API chain_draw(f32 dt);
extern "C" void CHAIN_API chain_exit();
