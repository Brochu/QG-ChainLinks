#pragma once

#define GEN_API __declspec(dllexport)

extern "C" void GEN_API generator_init();
extern "C" void GEN_API generator_stop();
