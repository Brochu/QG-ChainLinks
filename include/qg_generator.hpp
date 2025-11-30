#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "qg_types.hpp"
#include "qg_memory.hpp"

// CASE GENERATOR ====================

enum city_size : i8 { SIZE_SMALL, SIZE_MEDIUM, SIZE_LARGE, SIZE_METRO, SIZE_COUNT };
enum district_type : i8 { INDUSTRIAL, COMMERCIAL, RESIDENTIAL, NIGHT_LIFE, FINANCIAL, DISTRICT_COUNT };
enum landmark_type : i8 {
    //TODO: More variety
    // categorize by district type
    LOCATION_RESIDENTIAL,
    LOCATION_HOME = LOCATION_RESIDENTIAL,
    LOCATION_RESTAURANT,
    LOCATION_BANK,
    LOCATION_COUNT,
};

extern i8 size_to_districts[city_size::SIZE_COUNT];
extern i32 district_weights[district_type::DISTRICT_COUNT];
extern i8 size_to_landmarks[city_size::SIZE_COUNT];

struct district {
    i64 id;
    district_type type;
};

struct landmark {
    i64 id;
    i64 district;
};

struct case_gen {
    city_size size;
    i8 num_districts;
    district districts[16];

    i8 num_landmarks;
    landmark landmarks[128];
};

void case_gen_init(case_gen *ctx);

void case_gen_fondation(case_gen *ctx, city_size s, i32 seed);
void case_gen_population(case_gen *ctx);
void case_gen_motive(case_gen *ctx);
void case_gen_crime(case_gen *ctx);
void case_gen_planning(case_gen *ctx);
void case_gen_exec(case_gen *ctx);
void case_gen_hook(case_gen *ctx);
void case_gen_polish(case_gen *ctx);

void case_gen_clear(case_gen *ctx);

// NAME GENERATOR ====================

//TODO: Find a way to simplify this data structure; remove maps/vectors
//TODO: Each name_gen should have it's own memory pool for names
struct name_gen {
    std::unordered_map<std::string, std::unordered_map<char, i32>> counts;
};

void name_gen_train(name_gen *gen, const char *file_path);
void name_gen_next(name_gen *gen, u64 num, std::vector<std::string> *out);

void name_gen_district(name_gen *gen, u64 num, std::vector<std::string> *out);

struct name_cycle {
    mem_arena mem;
    std::vector<const char*> list;
    u64 step;
    u64 next;
};

void name_cycle_init(name_cycle *ctx, const char *file_path);
void name_cycle_next(name_cycle *ctx, u64 num, std::vector<std::string> *out);
void name_cycle_clear(name_cycle *ctx);
