#pragma once

#define CHAIN_API __declspec(dllexport)

extern "C" void CHAIN_API chain_init();
extern "C" void CHAIN_API chain_tick(float dt);
extern "C" void CHAIN_API chain_draw(float dt);
extern "C" void CHAIN_API chain_exit();
