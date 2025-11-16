#pragma once

#include <cstdint>

#include <unordered_map>
#include <string>
#include <vector>

// CASE GENERATOR ====================

enum city_size : int8_t { SIZE_SMALL, SIZE_MEDIUM, SIZE_LARGE, SIZE_METRO, SIZE_COUNT };
enum district_type : int8_t { INDUSTRIAL, COMMERCIAL, RESIDENTIAL, NIGHT_LIFE, FINANCIAL, DISTRICT_COUNT };
enum landmark_type : int8_t {
    //TODO: More variety
    // categorize by district type
    LOCATION_RESIDENTIAL,
    LOCATION_HOME = LOCATION_RESIDENTIAL,
    LOCATION_RESTAURANT,
    LOCATION_BANK,
    LOCATION_COUNT,
};

extern int8_t size_to_districts[city_size::SIZE_COUNT];
extern int8_t district_weights[district_type::DISTRICT_COUNT];
extern int8_t size_to_landmarks[city_size::SIZE_COUNT];

struct district {
    int64_t id;
    district_type type;
};

struct landmark {
    int64_t id;
    int64_t district;
};

struct case_gen {
    city_size size;
    int8_t num_districts;
    district districts[16];

    int8_t num_landmarks;
    landmark landmarks[128];
};

void case_gen_fondation(case_gen *ctx, city_size s, int32_t seed);
void case_gen_population(case_gen *ctx);
void case_gen_motive(case_gen *ctx);
void case_gen_crime(case_gen *ctx);
void case_gen_planning(case_gen *ctx);
void case_gen_exec(case_gen *ctx);
void case_gen_hook(case_gen *ctx);
void case_gen_polish(case_gen *ctx);

// NAME GENERATOR ====================

//TODO: Find a way to simplify this data structure; remove maps/vectors
struct name_gen {
    std::vector<std::string> starts;
    std::unordered_map<std::string, std::unordered_map<char, int8_t>> counts;
};

void name_gen_train(name_gen *gen);
void name_gen_next(name_gen *gen, size_t num, std::vector<std::string> *out);
